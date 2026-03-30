#ifndef BIT_BRIDGE_TCP_PROXY_HPP
#define BIT_BRIDGE_TCP_PROXY_HPP

#include "IRoutingStrategy.hpp"
#include "ServicePool.hpp"

#include <boost/asio.hpp>
#include <memory>
#include <string>

class TcpProxy {
public:
    TcpProxy(boost::asio::io_context &ioContext,
             const std::string &address,
             uint16_t port,
             ServicePool &pool,
             IRoutingStrategy &strategy,
             uint32_t connectTimeoutMs,
             uint32_t idleTimeoutMs);

    TcpProxy(const TcpProxy &) = delete;

    TcpProxy(TcpProxy &&) = delete;

    TcpProxy &operator=(const TcpProxy &) = delete;

    TcpProxy &operator=(TcpProxy &&) = delete;

    ~TcpProxy() = default;

    void Start();

    void Stop();

private:
    void AcceptNext();

    void OnAccept(boost::asio::ip::tcp::socket clientSocket,
                  const boost::system::error_code &ec);

    boost::asio::ip::tcp::acceptor m_acceptor;
    ServicePool &m_pool;
    IRoutingStrategy &m_strategy;
    uint32_t m_connectTimeoutMs;
    uint32_t m_idleTimeoutMs;
};

#endif //BIT_BRIDGE_TCP_PROXY_HPP