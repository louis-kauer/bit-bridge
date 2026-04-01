#ifndef BIT_BRIDGE_SESSION_HPP
#define BIT_BRIDGE_SESSION_HPP

#include "ServiceState.hpp"

#include <boost/asio.hpp>
#include <array>
#include <memory>

class Session : public std::enable_shared_from_this<Session> {
public:
    static constexpr size_t BUFFER_SIZE = 8192;

    Session(boost::asio::ip::tcp::socket clientSocket,
            ServiceState &backend,
            uint32_t connectTimeoutMs,
            uint32_t idleTimeoutMs);

    Session(const Session &) = delete;

    Session(Session &&) = delete;

    Session &operator=(const Session &) = delete;

    Session &operator=(Session &&) = delete;

    ~Session();

    void Start();

private:
    void ConnectToBackend();

    void OnConnected(const boost::system::error_code &ec);

    void ReadFromClient();

    void ReadFromBackend();

    void Shutdown();

    boost::asio::ip::tcp::socket m_clientSocket;
    boost::asio::ip::tcp::socket m_backendSocket;
    boost::asio::steady_timer m_connectTimer;
    boost::asio::steady_timer m_idleTimer;

    ServiceState &m_backend;

    uint32_t m_connectTimeoutMs;
    uint32_t m_idleTimeoutMs;

    std::array<char, BUFFER_SIZE> m_clientBuf{};
    std::array<char, BUFFER_SIZE> m_backendBuf{};

    bool m_shuttingDown{false};
};

#endif //BIT_BRIDGE_SESSION_HPP