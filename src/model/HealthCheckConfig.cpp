#include "HealthCheckConfig.hpp"

HealthCheckConfig::HealthCheckConfig()
    : m_enabled(true)
      , m_intervalMs(5000)
      , m_timeoutMs(2000)
      , m_unhealthyThreshold(3) {
}

HealthCheckConfig::HealthCheckConfig(bool enabled, uint32_t intervalMs, uint32_t timeoutMs, uint32_t unhealthyThreshold)
    : m_enabled(enabled)
      , m_intervalMs(intervalMs)
      , m_timeoutMs(timeoutMs)
      , m_unhealthyThreshold(unhealthyThreshold) {
}

bool HealthCheckConfig::GetEnabled() const { return m_enabled; }
uint32_t HealthCheckConfig::GetIntervalMs() const { return m_intervalMs; }
uint32_t HealthCheckConfig::GetTimeoutMs() const { return m_timeoutMs; }
uint32_t HealthCheckConfig::GetUnhealthyThreshold() const { return m_unhealthyThreshold; }

void HealthCheckConfig::SetEnabled(bool enabled) { m_enabled = enabled; }
void HealthCheckConfig::SetIntervalMs(uint32_t intervalMs) { m_intervalMs = intervalMs; }
void HealthCheckConfig::SetTimeoutMs(uint32_t timeoutMs) { m_timeoutMs = timeoutMs; }

void HealthCheckConfig::SetUnhealthyThreshold(uint32_t unhealthyThreshold) {
    m_unhealthyThreshold = unhealthyThreshold;
}