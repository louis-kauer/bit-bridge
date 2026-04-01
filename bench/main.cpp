// C++ benchmark driver for bit_bridge_lb.
// Spins up in-process mock TCP services, starts the LB binary as a subprocess,
// then hammers it with concurrent connections and reports latency/throughput stats.
//
// Usage:
//   bit_bridge_bench [--baseline] [--compare] [--algo p2c|consistent-hash|both]
//                    [--concurrency N] [--requests N] [--services N]
//                    [--threshold-pct N] [--baseline-file path]
//                    <lb-binary>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint> // NOLINT(misc-include-cleaner)
#include <functional>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <numeric>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

static constexpr uint16_t k_lbPort = 18090;
static constexpr uint16_t k_serviceBasePort = 19100;
static constexpr int k_connectTimeoutMs = 3000;
static constexpr int k_httpResponseBodyBytes = 128;

static constexpr auto k_ansiCyan = "\x{1b}[0;36m";
static constexpr auto k_ansiGreen = "\x{1b}[0;32m";
static constexpr auto k_ansiBoldYel = "\x{1b}[1;33m";
static constexpr auto k_ansiRed = "\x{1b}[0;31m";
static constexpr auto k_ansiBold = "\x{1b}[1m";
static constexpr auto k_ansiReset = "\x{1b}[0m";

struct BenchConfig {
    std::string m_lbBinary;
    std::string m_algo = "both";
    int m_concurrency = 50;
    int m_requests = 1000;
    int m_services = 5;
    int m_thresholdPct = 15;
    int m_backendDelayMs = 0;
    bool m_baseline = false;
    bool m_compare = false;
    std::string m_baselineFile = "benchmarks/baseline.json";
};

struct AlgoResult {
    std::string m_algo;
    int m_completed = 0;
    int m_errors = 0;
    int m_throughput = 0;
    int m_peakConnections = 0;
    int m_backendCount = 0;

    int64_t m_bytesReceived = 0;
    double m_avgMs = 0.0;
    double m_minMs = 0.0;
    double m_p50Ms = 0.0;
    double m_p95Ms = 0.0;
    double m_p99Ms = 0.0;
    double m_maxMs = 0.0;
};

struct ConnResult {
    bool m_ok = false;
    double m_ms = 0.0;
    int64_t m_bytes = 0;
};

template<typename... Args>
static void Log(std::format_string<Args...> fmt, Args &&... args) {
    std::println("{}[bench]{} {}", k_ansiCyan, k_ansiReset,
                 std::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
static void Success(std::format_string<Args...> fmt, Args &&... args) {
    std::println("{}[✓]{} {}", k_ansiGreen, k_ansiReset,
                 std::format(fmt, std::forward<Args>(args)...));
}

template<typename... Args>
static void Header(std::format_string<Args...> fmt, Args &&... args) {
    std::println("\n{}{}{}{}", k_ansiBold, k_ansiBoldYel,
                 std::format(fmt, std::forward<Args>(args)...), k_ansiReset);
}

template<typename... Args>
[[noreturn]] static void Die(std::format_string<Args...> fmt, Args &&... args) {
    std::println(stderr, "{}[error]{} {}", k_ansiRed, k_ansiReset,
                 std::format(fmt, std::forward<Args>(args)...));
    std::exit(1);
}

// Reads an HTTP request then responds with HTTP 200 + fixed-size body
// Optional delay simulates backend processing time
class EchoService {
public:
    EchoService(asio::io_context &ioc, uint16_t port, int delayMs)
        : m_ioc(ioc), m_acceptor(ioc), m_delayMs(delayMs) {
        m_acceptor.open(tcp::v4());
        m_acceptor.set_option(tcp::acceptor::reuse_address(true));
        m_acceptor.bind(tcp::endpoint(asio::ip::address_v4::loopback(), port));
        m_acceptor.listen();
        AcceptNext();
    }

    EchoService(const EchoService &) = delete;

    EchoService(EchoService &&) = delete;

    EchoService &operator=(const EchoService &) = delete;

    EchoService &operator=(EchoService &&) = delete;

    ~EchoService() {
        boost::system::error_code ignored;
        m_acceptor.close(ignored); // NOLINT(bugprone-unused-return-value)
    }

    [[nodiscard]] uint16_t Port() const { return m_acceptor.local_endpoint().port(); }

private:
    void AcceptNext() {
        m_acceptor.async_accept([this](const boost::system::error_code &ec, tcp::socket sock) {
            if (ec) { return; }
            HandleConn(std::make_shared<tcp::socket>(std::move(sock)));
            AcceptNext();
        });
    }

    void HandleConn(const std::shared_ptr<tcp::socket> &sock) {
        auto buf = std::make_shared<std::array<char, 4096> >();
        sock->async_read_some(asio::buffer(*buf),
                              [this, sock, buf](const boost::system::error_code &ec, size_t) { // NOLINT(bugprone-unused-return-value)
                                  if (ec) { return; }
                                  if (m_delayMs > 0) {
                                      auto timer = std::make_shared<asio::steady_timer>(m_ioc);
                                      timer->expires_after(std::chrono::milliseconds(m_delayMs));
                                      timer->async_wait([sock, timer](const boost::system::error_code &) {
                                          SendResponse(sock);
                                      });
                                  } else {
                                      SendResponse(sock);
                                  }
                              });
    }

    static void SendResponse(const std::shared_ptr<tcp::socket> &sock) {
        const std::string body(k_httpResponseBodyBytes, 'x');
        auto resp = std::make_shared<std::string>(
            std::format("HTTP/1.1 200 OK\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
                        k_httpResponseBodyBytes, body));
        asio::async_write(*sock, asio::buffer(*resp),
                          [sock, resp](const boost::system::error_code &, size_t) { // NOLINT(bugprone-unused-return-value)
                              boost::system::error_code ignored;
                              sock->shutdown(tcp::socket::shutdown_both, ignored);
                              sock->close(ignored); // NOLINT(bugprone-unused-return-value)
                          });
    }

    asio::io_context &m_ioc;
    tcp::acceptor m_acceptor;
    int m_delayMs;
};

[[nodiscard]] static std::string WriteTempConfig(
    const std::string &algo,
    uint16_t lbPort, // NOLINT(misc-unused-parameters) — kept as parameter for clarity
    const std::vector<uint16_t> &servicePorts) {
    const std::string path = std::format("/tmp/bb-cppbench-{}.yaml", algo);
    std::ofstream f(path);
    if (!f) { Die("cannot write temp config to {}", path); }

    std::println(f, "name: bench-{}", algo);
    std::println(f, R"(listenAddress: "127.0.0.1")");
    std::println(f, "listenPort: {}", lbPort);
    std::println(f, R"(routingAlgorithm: "{}")", algo);
    std::println(f, "services:");
    for (size_t i = 0; i < servicePorts.size(); ++i) {
        std::println(f, "  - name: service-{}", i + 1);
        std::println(f, R"(    ip: "127.0.0.1")");
        std::println(f, "    port: {}", servicePorts.at(i));
        std::println(f, "    weight: 1");
    }
    std::println(f, "healthCheck:");
    std::println(f, "  enabled: false");
    std::println(f, "connection:");
    std::println(f, "  maxPerService: 2048");
    std::println(f, "  idleTimeoutMs: 5000");
    std::println(f, "  connectTimeoutMs: 2000");

    return path;
}

[[nodiscard]] static pid_t StartLb(const std::string &binary, const std::string &config) {
    const pid_t pid = ::fork();
    if (pid < 0) { Die("fork failed"); }
    if (pid == 0) {
        if (const int devnull = ::open("/dev/null", O_WRONLY); devnull >= 0) { // NOLINT(cppcoreguidelines-pro-type-vararg)
            ::dup2(devnull, STDOUT_FILENO);
            ::dup2(devnull, STDERR_FILENO);
            ::close(devnull);
        }
        ::execlp(binary.c_str(), binary.c_str(), config.c_str(), nullptr); // NOLINT(cppcoreguidelines-pro-type-vararg)
        ::_exit(1);
    }
    return pid;
}

static void StopLb(pid_t pid) {
    if (pid <= 0) { return; }
    ::kill(pid, SIGTERM);
    int status = 0;
    ::waitpid(pid, &status, 0);
}

static void WaitPort(uint16_t port, int maxAttempts = 100) {
    const tcp::endpoint ep(asio::ip::address_v4::loopback(), port);
    for (int i = 0; i < maxAttempts; ++i) {
        asio::io_context ioc;
        tcp::socket sock(ioc);
        boost::system::error_code ec;
        sock.connect(ep, ec);
        if (!ec) { return; }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    Die("port {} never opened", port);
}

struct LoadContext {
    asio::io_context &m_ioc;
    asio::strand<asio::io_context::executor_type> m_strand;
    std::vector<ConnResult> &m_results;
    std::mutex &m_mu;
    std::atomic<int> &m_inflight;
    std::atomic<int> &m_peakInflight;
    std::atomic<int> &m_launched;
    std::atomic<int> &m_done;
    std::atomic<int64_t> &m_bytesReceived;
    tcp::endpoint m_ep;
    int m_total;
    int m_concurrency;
};

static void LaunchWave(LoadContext &ctx);

// Bundles per-connection read state to keep function signatures within param limits
struct ReadState {
    std::shared_ptr<tcp::socket> m_sock;
    std::shared_ptr<asio::steady_timer> m_timer;
    std::shared_ptr<std::vector<char> > m_buf;
    std::chrono::steady_clock::time_point m_t0;
    int64_t m_bytesRead = 0;
};

static void ReadResponse(LoadContext &ctx, const std::shared_ptr<ReadState> &state);

static void OnReadSome(LoadContext &ctx,
                       const std::shared_ptr<ReadState> &state,
                       const boost::system::error_code &ec,
                       size_t n) {
    if (!ec) {
        state->m_bytesRead += static_cast<int64_t>(n);
        ReadResponse(ctx, state);
        return;
    }
    state->m_timer->cancel();
    const double ms = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - state->m_t0).count(); // NOLINT(readability-isolate-declaration)
    const bool ok = (ec == asio::error::eof || ec == asio::error::connection_reset);
    {
        std::scoped_lock lock(ctx.m_mu);
        ctx.m_results.push_back({.m_ok = ok, .m_ms = ms, .m_bytes = state->m_bytesRead});
    }
    if (ok) { ctx.m_bytesReceived.fetch_add(state->m_bytesRead); }
    boost::system::error_code ignored;
    state->m_sock->close(ignored); // NOLINT(bugprone-unused-return-value)
    ctx.m_inflight.fetch_sub(1);
    ctx.m_done.fetch_add(1);
    LaunchWave(ctx);
    if (ctx.m_done.load() >= ctx.m_total) { ctx.m_ioc.stop(); }
}

// Reads the full HTTP response, accumulating bytes, then records the result.
static void ReadResponse(LoadContext &ctx, const std::shared_ptr<ReadState> &state) {
    state->m_sock->async_read_some(asio::buffer(*state->m_buf),
                                   asio::bind_executor(ctx.m_strand,
                                                       [&ctx, state](const boost::system::error_code &ec, size_t n) {
                                                           OnReadSome(ctx, state, ec, n);
                                                       }));
}

static void OnWriteDone(LoadContext &ctx,
                        const std::shared_ptr<tcp::socket> &sock,
                        const std::shared_ptr<asio::steady_timer> &timer,
                        std::chrono::steady_clock::time_point t0,
                        const boost::system::error_code &writeEc) {
    if (writeEc) {
        timer->cancel();
        const double ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - t0).count(); // NOLINT(readability-isolate-declaration)
        {
            std::scoped_lock lock(ctx.m_mu);
            ctx.m_results.push_back({.m_ok = false, .m_ms = ms, .m_bytes = 0});
        }
        boost::system::error_code ignored;
        sock->close(ignored); // NOLINT(bugprone-unused-return-value)
        ctx.m_inflight.fetch_sub(1);
        ctx.m_done.fetch_add(1);
        LaunchWave(ctx);
        if (ctx.m_done.load() >= ctx.m_total) { ctx.m_ioc.stop(); }
        return;
    }
    auto state = std::make_shared<ReadState>(ReadState{
        .m_sock = sock,
        .m_timer = timer,
        .m_buf = std::make_shared<std::vector<char> >(4096),
        .m_t0 = t0,
        .m_bytesRead = 0
    });
    ReadResponse(ctx, state);
}

static void OnConnect(LoadContext &ctx,
                      const std::shared_ptr<tcp::socket> &sock,
                      const std::shared_ptr<asio::steady_timer> &timer,
                      std::chrono::steady_clock::time_point t0,
                      const boost::system::error_code &ec) {
    if (ec) {
        timer->cancel();
        const double ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - t0).count(); // NOLINT(readability-isolate-declaration)
        {
            std::scoped_lock lock(ctx.m_mu);
            ctx.m_results.push_back({.m_ok = false, .m_ms = ms, .m_bytes = 0});
        }
        boost::system::error_code ignored;
        sock->close(ignored); // NOLINT(bugprone-unused-return-value)
        ctx.m_inflight.fetch_sub(1);
        ctx.m_done.fetch_add(1);
        LaunchWave(ctx);
        if (ctx.m_done.load() >= ctx.m_total) { ctx.m_ioc.stop(); }
        return;
    }

    // Cancel the timeout — connected successfully, response read handles its own lifetime
    timer->cancel();

    // Send a minimal HTTP GET request
    auto req = std::make_shared<std::string>("GET / HTTP/1.1\r\nHost: bench\r\nConnection: close\r\n\r\n");
    asio::async_write(*sock, asio::buffer(*req),
                      asio::bind_executor(ctx.m_strand,
                                          [&ctx, sock, timer, t0, req](const boost::system::error_code &writeEc, size_t) { // NOLINT(bugprone-unused-return-value)
                                              OnWriteDone(ctx, sock, timer, t0, writeEc);
                                          }));
}

static void LaunchWave(LoadContext &ctx) {
    if (ctx.m_launched.load() >= ctx.m_total) { return; }

    while (ctx.m_inflight.load() < ctx.m_concurrency && ctx.m_launched.load() < ctx.m_total) {
        const int current = ctx.m_inflight.fetch_add(1) + 1;
        ctx.m_launched.fetch_add(1);

        // Update peak high-water mark
        int peak = ctx.m_peakInflight.load();
        while (current > peak && !ctx.m_peakInflight.compare_exchange_weak(peak, current)) {
            // CAS retry — peak is updated by compare_exchange_weak on failure
        }

        auto sock = std::make_shared<tcp::socket>(ctx.m_ioc);
        auto timer = std::make_shared<asio::steady_timer>(ctx.m_ioc);
        const auto t0 = std::chrono::steady_clock::now();

        timer->expires_after(std::chrono::milliseconds(k_connectTimeoutMs));
        timer->async_wait([sock](const boost::system::error_code &ec) {
            if (ec) { return; } // cancelled — do nothing
            boost::system::error_code ignored;
            sock->close(ignored); // NOLINT(bugprone-unused-return-value)
        });

        sock->async_connect(ctx.m_ep,
                            asio::bind_executor(ctx.m_strand,
                                                [&ctx, sock, timer, t0](const boost::system::error_code &ec) {
                                                    OnConnect(ctx, sock, timer, t0, ec);
                                                }));
    }
}

struct LoadResult {
    std::vector<ConnResult> m_conns;
    int m_peakConnections = 0;
    int64_t m_bytesReceived = 0;
};

// NOLINTBEGIN(misc-unused-parameters)
[[nodiscard]] static LoadResult RunLoad(uint16_t lbPort, int total, int concurrency) {
    // NOLINTEND(misc-unused-parameters)
    asio::io_context ioc;
    std::vector<ConnResult> results;
    results.reserve(static_cast<size_t>(total));
    std::mutex mu;

    std::atomic inflight{0};
    std::atomic peakInflight{0};
    std::atomic launched{0};
    std::atomic done{0};
    std::atomic<int64_t> bytesReceived{0};

    LoadContext ctx{
        .m_ioc = ioc,
        .m_strand = asio::make_strand(ioc),
        .m_results = results, .m_mu = mu,
        .m_inflight = inflight, .m_peakInflight = peakInflight,
        .m_launched = launched, .m_done = done, .m_bytesReceived = bytesReceived,
        .m_ep = tcp::endpoint(asio::ip::address_v4::loopback(), lbPort),
        .m_total = total, .m_concurrency = concurrency
    };

    // work guard keeps ioc alive — ioc.stop() from within callbacks is the only exit point
    auto work = asio::make_work_guard(ioc);

    // Post the first wave then run the event loop on this thread
    asio::post(ioc, [&ctx]() { LaunchWave(ctx); });
    ioc.run();

    return {
        .m_conns = std::move(results), .m_peakConnections = peakInflight.load(), .m_bytesReceived = bytesReceived.load()
    };
}

[[nodiscard]] static AlgoResult ComputeStats(const std::string &algo,
                                             const LoadResult &load,
                                             double totalWallMs,
                                             int backendCount) {
    AlgoResult r;
    r.m_algo = algo;
    r.m_peakConnections = load.m_peakConnections;
    r.m_bytesReceived = load.m_bytesReceived;
    r.m_backendCount = backendCount;

    std::vector<double> latencies;
    latencies.reserve(load.m_conns.size());

    for (const auto &c: load.m_conns) {
        if (c.m_ok) {
            r.m_completed++;
            latencies.push_back(c.m_ms);
        } else {
            r.m_errors++;
        }
    }

    if (totalWallMs > 0) {
        r.m_throughput = static_cast<int>(r.m_completed * 1000.0 / totalWallMs);
    }

    if (latencies.empty()) { return r; }

    std::ranges::sort(latencies);
    const size_t n = latencies.size();

    r.m_minMs = latencies.front();
    r.m_maxMs = latencies.back();
    r.m_avgMs = std::accumulate(latencies.begin(), latencies.end(), 0.0) / static_cast<double>(n);
    r.m_p50Ms = latencies.at(n * 50 / 100);
    r.m_p95Ms = latencies.at(n * 95 / 100);
    r.m_p99Ms = latencies.at(n * 99 / 100);

    return r;
}

static void PrintResult(const AlgoResult &r) {
    const int total = r.m_completed + r.m_errors;
    const int errPct = total > 0 ? (r.m_errors * 100 / total) : 0;
    const double kbReceived = static_cast<double>(r.m_bytesReceived) / 1024.0;

    std::println("\n  {}{}{}", k_ansiBold, r.m_algo, k_ansiReset);
    std::println("  ──────────────────────  ────────────");
    std::println("  {:<22} {:>12}", "Backends", std::format("{} healthy", r.m_backendCount));
    std::println("  {:<22} {:>12}", "Completed", r.m_completed);
    std::println("  {:<22} {:>11}%", "Error rate", errPct);
    std::println("  {:<22} {:>12}", "Peak connections", r.m_peakConnections);
    std::println("  {:<22} {:>10}/s", "Throughput", r.m_throughput);
    std::println("  {:<22} {:>9.1f} KB", "Data received", kbReceived);
    std::println("  {:<22} {:>9.1f} ms", "Avg latency", r.m_avgMs);
    std::println("  {:<22} {:>9.1f} ms", "Min latency", r.m_minMs);
    std::println("  {:<22} {:>9.1f} ms", "p50 latency", r.m_p50Ms);
    std::println("  {:<22} {:>9.1f} ms", "p95 latency", r.m_p95Ms);
    std::println("  {:<22} {:>9.1f} ms", "p99 latency", r.m_p99Ms);
    std::println("  {:<22} {:>9.1f} ms", "Max latency", r.m_maxMs);
    std::println("  ──────────────────────  ────────────");
}

static void SaveBaseline(const BenchConfig &cfg, const std::vector<AlgoResult> &results) {
    std::filesystem::create_directories(
        std::filesystem::path(cfg.m_baselineFile).parent_path());

    std::ofstream f(cfg.m_baselineFile);
    if (!f) { Die("cannot write baseline to {}", cfg.m_baselineFile); }

    const auto now = std::chrono::system_clock::now();
    const std::string ts = std::format("{:%Y%m%d_%H%M%S}", now);

    std::println(f, "{{");
    std::println(f, R"(  "timestamp": "{}",)", ts);
    std::println(f, R"(  "binary": "{}",)", cfg.m_lbBinary);
    std::println(f, R"(  "concurrency": {},)", cfg.m_concurrency);
    std::println(f, R"(  "requests": {},)", cfg.m_requests);
    std::println(f, R"(  "services": {},)", cfg.m_services);
    std::println(f, R"(  "results": {{)");

    for (size_t i = 0; i < results.size(); ++i) {
        const auto &r = results.at(i);
        const int total = r.m_completed + r.m_errors;
        const int errPct = total > 0 ? (r.m_errors * 100 / total) : 0;
        const char *comma = (i + 1 < results.size()) ? "," : "";

        std::println(f, R"(    "{}": {{)", r.m_algo);
        std::println(f, R"(      "completed": {},)", r.m_completed);
        std::println(f, R"(      "errors": {},)", r.m_errors);
        std::println(f, R"(      "error_pct": {},)", errPct);
        std::println(f, R"(      "throughput_per_sec": {},)", r.m_throughput);
        std::println(f, R"(      "peak_connections": {},)", r.m_peakConnections);
        std::println(f, R"(      "bytes_received": {},)", r.m_bytesReceived);
        std::println(f, R"(      "avg_ms": {:.2f},)", r.m_avgMs);
        std::println(f, R"(      "min_ms": {:.2f},)", r.m_minMs);
        std::println(f, R"(      "p50_ms": {:.2f},)", r.m_p50Ms);
        std::println(f, R"(      "p95_ms": {:.2f},)", r.m_p95Ms);
        std::println(f, R"(      "p99_ms": {:.2f},)", r.m_p99Ms);
        std::println(f, R"(      "max_ms": {:.2f})", r.m_maxMs);
        std::println(f, "    }}{}", comma);
    }

    std::println(f, "  }}");
    std::println(f, "}}");

    Success("Baseline saved → {}", cfg.m_baselineFile);
}

// Simple line scan for throughput values — avoids pulling in a JSON parser.
[[nodiscard]] static bool CompareBaseline(const BenchConfig &cfg,
                                          const std::vector<AlgoResult> &current) {
    std::ifstream f(cfg.m_baselineFile);
    if (!f) {
        std::println("{}[warn]{} No baseline at {} — skipping comparison",
                     k_ansiBoldYel, k_ansiReset, cfg.m_baselineFile);
        return true;
    }

    const std::string content((std::istreambuf_iterator<char>(f)),
                              std::istreambuf_iterator<char>());

    bool allPassed = true;
    for (const auto &r: current) {
        const auto pos = content.find(std::format(R"("{}")", r.m_algo));
        if (pos == std::string::npos) {
            std::println("{}[warn]{} {} not in baseline — skipping",
                         k_ansiBoldYel, k_ansiReset, r.m_algo);
            continue;
        }

        constexpr std::string_view needle = R"("throughput_per_sec": )";
        const auto tpos = content.find(needle, pos);
        if (tpos == std::string::npos) { continue; }
        const int baseline = std::stoi(content.substr(tpos + needle.size(), 10));

        const double drop = (baseline > 0)
                                ? (1.0 - static_cast<double>(r.m_throughput) / baseline) * 100.0
                                : 0.0;

        if (drop > cfg.m_thresholdPct) {
            std::println(stderr,
                         "{}[FAIL]{} {} throughput regression: {}/s → {}/s ({:.0f}% drop, threshold {}%)",
                         k_ansiRed, k_ansiReset, r.m_algo, baseline, r.m_throughput, drop, cfg.m_thresholdPct);
            allPassed = false;
        } else {
            Success("{} throughput OK: {}/s (baseline {}/s, Δ {:.0f}%)",
                    r.m_algo, r.m_throughput, baseline, drop);
        }
    }
    return allPassed;
}

[[nodiscard]] static std::pair<std::vector<uint16_t>, std::vector<std::unique_ptr<EchoService> > >
SpawnEchoServices(asio::io_context &ioc, int count, int delayMs) {
    std::vector<uint16_t> ports;
    std::vector<std::unique_ptr<EchoService> > services;
    ports.reserve(static_cast<size_t>(count));
    services.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        auto svc = std::make_unique<EchoService>(ioc,
                                                 static_cast<uint16_t>(k_serviceBasePort + i),
                                                 delayMs);
        ports.push_back(svc->Port());
        services.push_back(std::move(svc));
    }
    return {std::move(ports), std::move(services)};
}

[[nodiscard]] static AlgoResult RunPhase(const BenchConfig &cfg,
                                         const std::string &algo,
                                         const std::vector<uint16_t> &servicePorts) {
    Header("─── {} ───", algo);
    Log("concurrency={}  requests={}  services={}  backend-delay={}ms",
        cfg.m_concurrency, cfg.m_requests, servicePorts.size(), cfg.m_backendDelayMs);

    const std::string cfgPath = WriteTempConfig(algo, k_lbPort, servicePorts);
    const pid_t lbPid = StartLb(cfg.m_lbBinary, cfgPath);
    WaitPort(k_lbPort);
    Success("LB started (pid={})  backends={} healthy", lbPid, servicePorts.size());

    const auto t0 = std::chrono::steady_clock::now();
    LoadResult load = RunLoad(k_lbPort, cfg.m_requests, cfg.m_concurrency);
    const auto t1 = std::chrono::steady_clock::now();
    const double wallMs = std::chrono::duration<double, std::milli>(t1 - t0).count(); // NOLINT(readability-isolate-declaration)

    StopLb(lbPid);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    AlgoResult r = ComputeStats(algo, load, wallMs, static_cast<int>(servicePorts.size()));
    PrintResult(r);
    return r;
}

[[nodiscard]] static BenchConfig ParseArgs(int argc, char **argv) {
    BenchConfig cfg;
    const std::span<char *> args(argv + 1, static_cast<size_t>(argc - 1));
    auto it = args.begin();

    auto nextInt = [&it, &args](const char *flag) {
        ++it;
        if (it == args.end()) { Die("{} requires an argument", flag); }
        return std::stoi(*it);
    };
    auto nextStr = [&it, &args](const char *flag) -> std::string_view {
        ++it;
        if (it == args.end()) { Die("{} requires an argument", flag); }
        return *it;
    };

    const std::unordered_map<std::string_view, std::function<void()> > handlers = {
        {"--baseline", [&cfg] { cfg.m_baseline = true; }},
        {"--compare", [&cfg] { cfg.m_compare = true; }},
        {"--algo", [&cfg, &nextStr] { cfg.m_algo = nextStr("--algo"); }},
        {"--concurrency", [&cfg, &nextInt] { cfg.m_concurrency = nextInt("--concurrency"); }},
        {"--requests", [&cfg, &nextInt] { cfg.m_requests = nextInt("--requests"); }},
        {"--services", [&cfg, &nextInt] { cfg.m_services = nextInt("--services"); }},
        {"--threshold-pct", [&cfg, &nextInt] { cfg.m_thresholdPct = nextInt("--threshold-pct"); }},
        {"--baseline-file", [&cfg, &nextStr] { cfg.m_baselineFile = nextStr("--baseline-file"); }},
        {"--backend-delay-ms", [&cfg, &nextInt] { cfg.m_backendDelayMs = nextInt("--backend-delay-ms"); }},
    };

    while (it != args.end()) {
        const std::string_view arg = *it;
        if (const auto h = handlers.find(arg); h != handlers.end()) {
            h->second();
        } else if (!arg.starts_with('-') && cfg.m_lbBinary.empty()) {
            cfg.m_lbBinary = arg;
        }
        ++it;
    }

    if (cfg.m_lbBinary.empty()) {
        Die("Usage: bit_bridge_bench [options] <lb-binary>\n"
            "  --baseline            Save results as new baseline\n"
            "  --compare             Compare against baseline, exit 1 on regression\n"
            "  --algo                p2c|consistent-hash|both (default: both)\n"
            "  --concurrency N       Parallel connections (default: 50)\n"
            "  --requests N          Total requests per algo (default: 1000)\n"
            "  --services N          Backend count (default: 5)\n"
            "  --backend-delay-ms N  Simulated backend processing delay (default: 0)\n"
            "  --threshold-pct N     Regression threshold % (default: 15)\n"
            "  --baseline-file P     Baseline JSON path (default: benchmarks/baseline.json)");
    }
    if (!std::filesystem::exists(cfg.m_lbBinary)) {
        Die("LB binary not found: {}", cfg.m_lbBinary);
    }

    return cfg;
}

int main(int argc, char **argv) {
    const BenchConfig cfg = ParseArgs(argc, argv);

    std::vector<std::string> algos;
    if (cfg.m_algo == "p2c") { // NOLINT(bugprone-branch-clone)
        algos = {"p2c"};
    } else if (cfg.m_algo == "consistent-hash") {
        algos = {"consistent-hash"};
    } else {
        algos = {"p2c", "consistent-hash"};
    }

    Header("═══════════════════════════════════════════════════════════");
    Header("  Bit Bridge LB — C++ Benchmark");
    Header("═══════════════════════════════════════════════════════════");
    Log("Binary:      {}", cfg.m_lbBinary);
    Log("Concurrency: {}", cfg.m_concurrency);
    Log("Requests:    {} per algorithm", cfg.m_requests);
    if (cfg.m_baseline) { Log("Mode:        save baseline → {}", cfg.m_baselineFile); }
    if (cfg.m_compare) { Log("Mode:        compare against {}", cfg.m_baselineFile); }

    asio::io_context mockIoc;
    auto mockWork = asio::make_work_guard(mockIoc);
    std::jthread mockThread([&mockIoc]() { mockIoc.run(); });

    auto [servicePorts, echoServices] = SpawnEchoServices(mockIoc, cfg.m_services, cfg.m_backendDelayMs);
    Success("Echo services ready on ports {}–{}", servicePorts.front(), servicePorts.back());

    std::vector<AlgoResult> results;
    results.reserve(algos.size());
    for (const auto &algo: algos) {
        results.push_back(RunPhase(cfg, algo, servicePorts));
        if (&algo != &algos.back()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    mockWork.reset();
    mockIoc.stop();

    Header("═══════════════════════════════════════════════════════════");
    Header("  Results Summary");
    Header("═══════════════════════════════════════════════════════════");
    for (const auto &r: results) {
        const int errPct = (r.m_completed + r.m_errors) > 0
                               ? r.m_errors * 100 / (r.m_completed + r.m_errors)
                               : 0;
        std::println("  {:<20}  {:>6}/s   p99 {:>5.1f} ms   errors {}%",
                     r.m_algo, r.m_throughput, r.m_p99Ms, errPct);
    }
    std::println("");

    if (cfg.m_baseline) { SaveBaseline(cfg, results); }
    if (cfg.m_compare && !CompareBaseline(cfg, results)) { return 1; }
    if (cfg.m_compare) { Success("All benchmarks within threshold"); }

    return 0;
}
