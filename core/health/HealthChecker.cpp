#include "HealthChecker.hpp"

#include <print>
#include <cstdio>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

HealthChecker::HealthChecker(asio::io_context &ioContext,
                             ServicePool &pool,
                             const HealthCheckConfig &config)
    : m_ioContext(ioContext)
      , m_pool(pool)
      , m_config(config)
      , m_timer(ioContext) {
}

void HealthChecker::Start() {
    ScheduleNextRound();
}

void HealthChecker::Stop() {
    m_stopped = true;
    m_timer.cancel();
}

void HealthChecker::ScheduleNextRound() {
    if (m_stopped) {
        return;
    }
    m_timer.expires_after(std::chrono::milliseconds(m_config.GetIntervalMs()));
    m_timer.async_wait([this](const boost::system::error_code &ec) {
        if (ec || m_stopped) {
            return;
        }
        ProbeAll();
        ScheduleNextRound();
    });
}

void HealthChecker::ProbeAll() {
    for (size_t i = 0; i < m_pool.GetSize(); ++i) {
        ProbeOne(i);
    }
}

void HealthChecker::OnProbeResult(const size_t index,
                                  const std::shared_ptr<tcp::socket> &socket,
                                  const boost::system::error_code &ec) {
    ServiceState &svc = m_pool.GetService(index);
    if (ec) {
        svc.IncrementFailures();
        if (const bool thresholdReached = svc.GetConsecutiveFailures() >= m_config.GetUnhealthyThreshold(); // NOLINT(cppcoreguidelines-init-variables)
            thresholdReached && svc.IsHealthy()) {
            std::println(stderr, "HealthChecker: {} marked unhealthy", svc.GetNode().GetName());
            svc.SetHealthy(false);
        }
        return;
    }

    if (!svc.IsHealthy()) {
        std::println("HealthChecker: {} recovered", svc.GetNode().GetName());
    }
    svc.ResetFailures();
    svc.SetHealthy(true);
    try {
        socket->close();
    } catch (const boost::system::system_error &e) {
        std::println(stderr, "HealthChecker: close error: {}", e.what());
    }
}

void HealthChecker::ProbeOne(size_t index) {
    const auto &node = m_pool.GetService(index).GetNode();
    auto socket = std::make_shared<tcp::socket>(m_ioContext);
    auto timer = std::make_shared<asio::steady_timer>(m_ioContext);

    timer->expires_after(std::chrono::milliseconds(m_config.GetTimeoutMs()));
    timer->async_wait([socket](const boost::system::error_code &ec) {
        if (ec) {
            return; // timer cancelled — connect succeeded or probe was stopped
        }
        try {
            socket->close();
        } catch (const boost::system::system_error &e) {
            std::println(stderr, "HealthChecker: timeout close error: {}", e.what());
        }
    });

    const tcp::endpoint endpoint(asio::ip::make_address(node.GetIp()), node.GetPort());
    socket->async_connect(endpoint,
                          [this, index, socket, timer](const boost::system::error_code &ec) {
                              timer->cancel();
                              OnProbeResult(index, socket, ec);
                          });
}
