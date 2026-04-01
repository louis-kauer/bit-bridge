#include "TcpProxy.hpp"
#include "ServicePool.hpp"
#include "Session.hpp"

#include <print>
#include <cstdio>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

TcpProxy::TcpProxy(asio::io_context &ioContext,
                   const std::string &address,
                   const uint16_t port,
                   ServicePool &pool,
                   IRoutingStrategy &strategy,
                   const uint32_t connectTimeoutMs,
                   const uint32_t idleTimeoutMs)
    : m_acceptor(ioContext, tcp::endpoint(asio::ip::make_address(address), port))
      , m_pool(pool)
      , m_strategy(strategy)
      , m_connectTimeoutMs(connectTimeoutMs)
      , m_idleTimeoutMs(idleTimeoutMs) {
}

void TcpProxy::Start() {
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    AcceptNext();
}

void TcpProxy::Stop() {
    try {
        m_acceptor.close();
    } catch (const boost::system::system_error &e) {
        std::println(stderr, "TcpProxy: acceptor close error: {}", e.what());
    }
}

void TcpProxy::AcceptNext() {
    m_acceptor.async_accept(
        [this](const boost::system::error_code &ec, tcp::socket clientSocket) {
            OnAccept(std::move(clientSocket), ec);
        });
}

void TcpProxy::OnAccept(tcp::socket clientSocket, const boost::system::error_code &ec) {
    if (ec == asio::error::operation_aborted) {
        return;
    }

    if (ec) {
        std::println(stderr, "TcpProxy: accept error: {}", ec.message());
        AcceptNext();
        return;
    }

    const auto clientEndpoint = clientSocket.remote_endpoint();
    const std::string routingKey = clientEndpoint.address().to_string();

    const auto result = m_strategy.SelectService(m_pool, routingKey);

    if (!result.has_value()) {
        std::println(stderr, "TcpProxy: no healthy backend for {}, dropping connection", routingKey);
        try {
            clientSocket.close();
        } catch (const boost::system::system_error &e) {
            std::println(stderr, "TcpProxy: close error: {}", e.what());
        }
        AcceptNext();
        return;
    }

    const size_t backendIdx = *result;
    ServiceState &backend = m_pool.GetService(backendIdx);

    auto session = std::make_shared<Session>(
        std::move(clientSocket),
        backend,
        m_connectTimeoutMs,
        m_idleTimeoutMs
    );
    backend.IncrementConnections();
    session->Start();

    AcceptNext();
}
