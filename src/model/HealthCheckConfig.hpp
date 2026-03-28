#ifndef BIT_BRIDGE_HEALTH_CHECK_CONFIG_HPP
#define BIT_BRIDGE_HEALTH_CHECK_CONFIG_HPP

#include <cstdint>

class HealthCheckConfig {
public:
    HealthCheckConfig();

    HealthCheckConfig(bool enabled, uint32_t intervalMs, uint32_t timeoutMs, uint32_t unhealthyThreshold);

    HealthCheckConfig(const HealthCheckConfig &other) = default;

    HealthCheckConfig(HealthCheckConfig &&other) noexcept = default;

    ~HealthCheckConfig() = default;

    HealthCheckConfig &operator=(const HealthCheckConfig &other) = default;

    HealthCheckConfig &operator=(HealthCheckConfig &&other) noexcept = default;

    [[nodiscard]] bool GetEnabled() const;

    [[nodiscard]] uint32_t GetIntervalMs() const;

    [[nodiscard]] uint32_t GetTimeoutMs() const;

    [[nodiscard]] uint32_t GetUnhealthyThreshold() const;

    void SetEnabled(bool enabled);

    void SetIntervalMs(uint32_t intervalMs);

    void SetTimeoutMs(uint32_t timeoutMs);

    void SetUnhealthyThreshold(uint32_t unhealthyThreshold);

private:
    bool m_enabled = true;
    uint32_t m_intervalMs = 0;
    uint32_t m_timeoutMs = 0;
    uint32_t m_unhealthyThreshold = 0;
};

#endif //BIT_BRIDGE_HEALTH_CHECK_CONFIG_HPP