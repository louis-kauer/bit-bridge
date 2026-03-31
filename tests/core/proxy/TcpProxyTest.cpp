#include <gtest/gtest.h>
#include "TcpProxy.hpp"
#include "P2CStrategy.hpp"
#include "ServicePool.hpp"
#include "ServiceNode.hpp"

#include <boost/asio.hpp>
#include <thread>
#include <array>
#include <memory>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class TcpProxyTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_backendAcceptor.open(tcp::v4());
        m_backendAcceptor.set_option(tcp::acceptor::reuse_address(true));
        m_backendAcceptor.bind(tcp::endpoint(asio::ip::address_v4::loopback(), 0));
        m_backendAcceptor.listen();
        m_backendPort = m_backendAcceptor.local_endpoint().port();

        // Find a free proxy port
        tcp::acceptor probe(m_ioContext);
        probe.open(tcp::v4());
        probe.bind(tcp::endpoint(asio::ip::address_v4::loopback(), 0));
        m_proxyPort = probe.local_endpoint().port();
        probe.close();
    }

    void TearDown() override {
        m_ioContext.stop();
    }

private:
    asio::io_context m_ioContext;
    tcp::acceptor m_backendAcceptor{m_ioContext};
    uint16_t m_backendPort{0};
    uint16_t m_proxyPort{0};

    friend class TcpProxyTest_AcceptsAndRoutesToBackend_Test;
    friend class TcpProxyTest_DropsConnectionWhenNoHealthyBackend_Test;
    friend class TcpProxyTest_StopClosesAcceptor_Test;
};

TEST_F(TcpProxyTest, AcceptsAndRoutesToBackend) {
    std::vector<ServiceNode> nodes{{"backend", "127.0.0.1", m_backendPort, 1}};
    ServicePool pool(nodes);
    P2CStrategy strategy;

    TcpProxy proxy(m_ioContext, "127.0.0.1", m_proxyPort, pool, strategy, 1000, 5000);
    proxy.Start();

    bool backendGotConnection = false;
    m_backendAcceptor.async_accept([&backendGotConnection](const boost::system::error_code &ec, tcp::socket) {
        if (!ec) {
            backendGotConnection = true;
        }
    });

    tcp::socket client(m_ioContext);
    client.async_connect(tcp::endpoint(asio::ip::address_v4::loopback(), m_proxyPort),
                         [](const boost::system::error_code &ec) { (void) ec; });

    m_ioContext.run_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(backendGotConnection);
}

TEST_F(TcpProxyTest, DropsConnectionWhenNoHealthyBackend) {
    std::vector<ServiceNode> nodes{{"backend", "127.0.0.1", m_backendPort, 1}};
    ServicePool pool(nodes);
    pool.GetService(0).SetHealthy(false);
    P2CStrategy strategy;

    TcpProxy proxy(m_ioContext, "127.0.0.1", m_proxyPort, pool, strategy, 1000, 5000);
    proxy.Start();

    bool clientConnectError = false;
    auto client = std::make_shared<tcp::socket>(m_ioContext);
    client->async_connect(tcp::endpoint(asio::ip::address_v4::loopback(), m_proxyPort),
                          [&clientConnectError, client](const boost::system::error_code &ec) {
                              if (ec) { return; }
                              // connected to proxy — wait for it to close
                              auto buf = std::make_shared<std::array<char, 16> >();
                              client->async_read_some(asio::buffer(*buf),
                                                      [&clientConnectError](
                                                  const boost::system::error_code &readEc, size_t) {
                                                          clientConnectError = (readEc == asio::error::eof ||
                                                              readEc == asio::error::connection_reset);
                                                      });
                          });

    m_ioContext.run_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(clientConnectError);
}

TEST_F(TcpProxyTest, StopClosesAcceptor) {
    std::vector<ServiceNode> nodes{{"backend", "127.0.0.1", m_backendPort, 1}};
    ServicePool pool(nodes);
    P2CStrategy strategy;

    TcpProxy proxy(m_ioContext, "127.0.0.1", m_proxyPort, pool, strategy, 1000, 5000);
    proxy.Start();
    proxy.Stop();

    // After stop, new connections should be refused
    bool connectFailed = false;
    tcp::socket client(m_ioContext);
    client.async_connect(tcp::endpoint(asio::ip::address_v4::loopback(), m_proxyPort),
                         [&connectFailed](const boost::system::error_code &ec) {
                             connectFailed = ec.operator bool();
                         });

    m_ioContext.run_for(std::chrono::milliseconds(200));
    EXPECT_TRUE(connectFailed);
}