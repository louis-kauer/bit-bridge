#include <gtest/gtest.h>
#include "Session.hpp"
#include "ServiceState.hpp"
#include "ServiceNode.hpp"

#include <boost/asio.hpp>
#include <string>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class SessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_backendAcceptor.open(tcp::v4());
        m_backendAcceptor.set_option(tcp::acceptor::reuse_address(true));
        m_backendAcceptor.bind(tcp::endpoint(asio::ip::address_v4::loopback(), 0));
        m_backendAcceptor.listen();
        m_backendPort = m_backendAcceptor.local_endpoint().port();
    }

    void TearDown() override {
        m_ioContext.stop();
    }

private:
    [[nodiscard]] ServiceNode MakeNode() const {
        return {"test-backend", "127.0.0.1", m_backendPort, 1};
    }

    asio::io_context m_ioContext;
    tcp::acceptor m_backendAcceptor{m_ioContext};
    uint16_t m_backendPort{0};

    friend class SessionTest_DataFlowsClientToBackend_Test;
    friend class SessionTest_ConnectionCounterDecrementedOnDestruction_Test;
    friend class SessionTest_ConnectFailureDecrementsCounter_Test;
};

TEST_F(SessionTest, DataFlowsClientToBackend) {
    ServiceState backend(MakeNode());
    backend.IncrementConnections();

    m_backendAcceptor.async_accept(
        [](const boost::system::error_code &ec, tcp::socket backendConn) {
            if (ec) { return; }
            auto buf = std::make_shared<std::array<char, 64> >();
            auto conn = std::make_shared<tcp::socket>(std::move(backendConn));
            conn->async_read_some(asio::buffer(*buf),
                                  [buf, conn](const boost::system::error_code &readEc, size_t) { (void) readEc; });
        });

    tcp::socket clientEnd(m_ioContext);
    clientEnd.open(tcp::v4());
    clientEnd.bind(tcp::endpoint(asio::ip::address_v4::loopback(), 0));

    auto session = std::make_shared<Session>(std::move(clientEnd), backend, 1000, 5000);
    session->Start();

    m_ioContext.run_for(std::chrono::milliseconds(200));
    EXPECT_EQ(backend.GetActiveConnections(), 1);
}

TEST_F(SessionTest, ConnectionCounterDecrementedOnDestruction) {
    ServiceState backend(MakeNode());
    backend.IncrementConnections();
    EXPECT_EQ(backend.GetActiveConnections(), 1);

    {
        tcp::socket clientSock(m_ioContext);
        auto session = std::make_shared<Session>(std::move(clientSock), backend, 1, 1);
        // session destroyed here
    }

    EXPECT_EQ(backend.GetActiveConnections(), 0);
}

TEST_F(SessionTest, ConnectFailureDecrementsCounter) {
    ServiceNode unreachable{"unreachable", "127.0.0.1", 19999, 1};
    ServiceState backend(unreachable);
    backend.IncrementConnections();

    tcp::socket clientSock(m_ioContext);
    clientSock.open(tcp::v4());
    clientSock.bind(tcp::endpoint(asio::ip::address_v4::loopback(), 0));

    {
        auto session = std::make_shared<Session>(std::move(clientSock), backend, 200, 5000);
        session->Start();
        m_ioContext.run();
    }

    EXPECT_EQ(backend.GetActiveConnections(), 0);
}