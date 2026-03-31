#include "Session.hpp"

#include <print>
#include <cstdio>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

Session::Session(tcp::socket clientSocket,
                 ServiceState &backend,
                 const uint32_t connectTimeoutMs,
                 const uint32_t idleTimeoutMs)
    : m_clientSocket(std::move(clientSocket))
      , m_backendSocket(m_clientSocket.get_executor())
      , m_connectTimer(m_clientSocket.get_executor())
      , m_idleTimer(m_clientSocket.get_executor())
      , m_backend(backend)
      , m_connectTimeoutMs(connectTimeoutMs)
      , m_idleTimeoutMs(idleTimeoutMs) {
}

Session::~Session() {
    m_backend.DecrementConnections();
}

void Session::Start() {
    ConnectToBackend();
}

void Session::ConnectToBackend() {
    const auto &node = m_backend.GetNode();
    const tcp::endpoint endpoint(asio::ip::make_address(node.GetIp()), node.GetPort());

    m_connectTimer.expires_after(std::chrono::milliseconds(m_connectTimeoutMs));
    m_connectTimer.async_wait([self = shared_from_this()](const boost::system::error_code &ec) {
        if (!ec) {
            self->Shutdown();
        }
    });

    m_backendSocket.async_connect(endpoint,
                                  [self = shared_from_this()](const boost::system::error_code &ec) {
                                      self->OnConnected(ec);
                                  });
}

void Session::OnConnected(const boost::system::error_code &ec) {
    m_connectTimer.cancel();

    if (ec) {
        std::println(stderr, "Session: backend connect failed: {}", ec.message());
        Shutdown();
        return;
    }

    ReadFromClient();
    ReadFromBackend();
}

void Session::ReadFromClient() {
    if (m_idleTimeoutMs > 0) {
        m_idleTimer.expires_after(std::chrono::milliseconds(m_idleTimeoutMs));
        m_idleTimer.async_wait([self = shared_from_this()](const boost::system::error_code &ec) {
            if (!ec) {
                self->Shutdown();
            }
        });
    }

    m_clientSocket.async_read_some(
        asio::buffer(m_clientBuf),
        [self = shared_from_this()](const boost::system::error_code &ec, const size_t bytes) {
            if (ec) {
                self->Shutdown();
                return;
            }
            asio::async_write(self->m_backendSocket,
                              asio::buffer(self->m_clientBuf.data(), bytes),
                              [self](const boost::system::error_code &writeEc, size_t) {
                                  if (writeEc) {
                                      self->Shutdown();
                                      return;
                                  }
                                  self->ReadFromClient();
                              });
        });
}

void Session::ReadFromBackend() {
    m_backendSocket.async_read_some(
        asio::buffer(m_backendBuf),
        [self = shared_from_this()](const boost::system::error_code &ec, const size_t bytes) {
            if (ec) {
                self->Shutdown();
                return;
            }
            asio::async_write(self->m_clientSocket,
                              asio::buffer(self->m_backendBuf.data(), bytes),
                              [self](const boost::system::error_code &writeEc, size_t) {
                                  if (writeEc) {
                                      self->Shutdown();
                                      return;
                                  }
                                  self->ReadFromBackend();
                              });
        });
}

void Session::Shutdown() {
    if (m_shuttingDown) {
        return;
    }
    m_shuttingDown = true;

    m_connectTimer.cancel();
    m_idleTimer.cancel();

    try {
        m_clientSocket.shutdown(tcp::socket::shutdown_both);
        m_clientSocket.close();
    } catch (const boost::system::system_error &e) {
        std::println(stderr, "Session: client socket close error: {}", e.what());
    }

    try {
        m_backendSocket.shutdown(tcp::socket::shutdown_both);
        m_backendSocket.close();
    } catch (const boost::system::system_error &e) {
        std::println(stderr, "Session: backend socket close error: {}", e.what());
    }
}
