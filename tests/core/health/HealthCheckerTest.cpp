#include <gtest/gtest.h>
#include "HealthChecker.hpp"
#include "ServicePool.hpp"
#include "ServiceNode.hpp"
#include "HealthCheckConfig.hpp"

#include <boost/asio.hpp>
#include <thread>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class HealthCheckerTest : public ::testing::Test {
public:
    [[nodiscard]] static HealthCheckConfig MakeConfig(uint32_t intervalMs,
                                                      uint32_t timeoutMs,
                                                      uint32_t threshold) {
        return {true, intervalMs, timeoutMs, threshold};
    }

protected:
    void TearDown() override {
        m_ioContext.stop();
    }

private:
    asio::io_context m_ioContext;

    friend class HealthCheckerTest_MarksBackendUnhealthyWhenUnreachable_Test;
    friend class HealthCheckerTest_KeepsBackendHealthyWhenReachable_Test;
    friend class HealthCheckerTest_RecoverBackendAfterComingBackUp_Test;
    friend class HealthCheckerTest_StopPreventsfurtherProbes_Test;
};

TEST_F(HealthCheckerTest, MarksBackendUnhealthyWhenUnreachable) {
    std::vector<ServiceNode> nodes{{"unreachable", "127.0.0.1", 19998, 1}};
    ServicePool pool(nodes);

    HealthChecker checker(m_ioContext, pool, MakeConfig(50, 50, 2));
    checker.Start();

    m_ioContext.run_for(std::chrono::milliseconds(500));

    EXPECT_FALSE(pool.GetService(0).IsHealthy());
    EXPECT_GE(pool.GetService(0).GetConsecutiveFailures(), 2);
}

TEST_F(HealthCheckerTest, KeepsBackendHealthyWhenReachable) {
    tcp::acceptor backend(m_ioContext);
    backend.open(tcp::v4());
    backend.set_option(tcp::acceptor::reuse_address(true));
    backend.bind(tcp::endpoint(asio::ip::address_v4::loopback(), 0));
    backend.listen();
    const uint16_t port = backend.local_endpoint().port();

    // Accept and immediately close probe connections
    std::function<void()> acceptLoop;
    acceptLoop = [&backend, &acceptLoop]() {
        backend.async_accept([&acceptLoop](const boost::system::error_code &ec, tcp::socket conn) {
            if (ec) { return; }
            conn.close();
            acceptLoop();
        });
    };
    acceptLoop();

    std::vector<ServiceNode> nodes{{"reachable", "127.0.0.1", port, 1}};
    ServicePool pool(nodes);

    HealthChecker checker(m_ioContext, pool, MakeConfig(50, 100, 3));
    checker.Start();

    m_ioContext.run_for(std::chrono::milliseconds(400));

    EXPECT_TRUE(pool.GetService(0).IsHealthy());
    EXPECT_EQ(pool.GetService(0).GetConsecutiveFailures(), 0);
}

TEST_F(HealthCheckerTest, RecoverBackendAfterComingBackUp) {
    // Start with no listener — backend is down
    std::vector<ServiceNode> nodes{{"recovering", "127.0.0.1", 19997, 1}};
    ServicePool pool(nodes);

    HealthChecker checker(m_ioContext, pool, MakeConfig(50, 50, 2));
    checker.Start();

    // Let it fail enough times to go unhealthy
    m_ioContext.run_for(std::chrono::milliseconds(300));
    EXPECT_FALSE(pool.GetService(0).IsHealthy());

    // Now bring up a listener on that port
    tcp::acceptor backend(m_ioContext);
    backend.open(tcp::v4());
    backend.set_option(tcp::acceptor::reuse_address(true));
    backend.bind(tcp::endpoint(asio::ip::address_v4::loopback(), 19997));
    backend.listen();

    std::function<void()> acceptLoop;
    acceptLoop = [&backend, &acceptLoop]() {
        backend.async_accept([&acceptLoop](const boost::system::error_code &ec, tcp::socket conn) {
            if (ec) { return; }
            conn.close();
            acceptLoop();
        });
    };
    acceptLoop();

    m_ioContext.run_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(pool.GetService(0).IsHealthy());
    EXPECT_EQ(pool.GetService(0).GetConsecutiveFailures(), 0);
}

TEST_F(HealthCheckerTest, StopPreventsfurtherProbes) {
    std::vector<ServiceNode> nodes{{"backend", "127.0.0.1", 19996, 1}};
    ServicePool pool(nodes);

    HealthChecker checker(m_ioContext, pool, MakeConfig(50, 50, 10));
    checker.Start();

    m_ioContext.run_for(std::chrono::milliseconds(120));
    const size_t failuresBeforeStop = pool.GetService(0).GetConsecutiveFailures();

    checker.Stop();
    m_ioContext.run_for(std::chrono::milliseconds(200));

    EXPECT_EQ(pool.GetService(0).GetConsecutiveFailures(), failuresBeforeStop);
}
