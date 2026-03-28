#ifndef BIT_BRIDGE_CONNECTION_CONFIG_HPP
#define BIT_BRIDGE_CONNECTION_CONFIG_HPP

#include <cstdint>

class ConnectionConfig {
public:
    ConnectionConfig();

    ConnectionConfig(uint32_t maxPerService, uint32_t idleTimeoutMs, uint32_t connectTimeoutMs);

    ConnectionConfig(const ConnectionConfig &other) = default;

    ConnectionConfig(ConnectionConfig &&other) noexcept = default;

    ~ConnectionConfig() = default;

    ConnectionConfig &operator=(const ConnectionConfig &other) = default;

    ConnectionConfig &operator=(ConnectionConfig &&other) noexcept = default;

    [[nodiscard]] uint32_t GetMaxPerService() const;

    [[nodiscard]] uint32_t GetIdleTimeoutMs() const;

    [[nodiscard]] uint32_t GetConnectTimeoutMs() const;

    void SetMaxPerService(uint32_t maxPerService);

    void SetIdleTimeoutMs(uint32_t idleTimeoutMs);

    void SetConnectTimeoutMs(uint32_t connectTimeoutMs);

private:
    uint32_t m_maxPerService = 0;
    uint32_t m_idleTimeoutMs = 0;
    uint32_t m_connectTimeoutMs = 0;
};

#endif //BIT_BRIDGE_CONNECTION_CONFIG_HPP