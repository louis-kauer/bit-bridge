#include "ConnectionConfig.hpp"

ConnectionConfig::ConnectionConfig()
    : m_maxPerService(1024)
      , m_idleTimeoutMs(30000)
      , m_connectTimeoutMs(5000) {
}

ConnectionConfig::ConnectionConfig(uint32_t maxPerService, uint32_t idleTimeoutMs, uint32_t connectTimeoutMs)
    : m_maxPerService(maxPerService)
      , m_idleTimeoutMs(idleTimeoutMs)
      , m_connectTimeoutMs(connectTimeoutMs) {
}

uint32_t ConnectionConfig::GetMaxPerService() const { return m_maxPerService; }
uint32_t ConnectionConfig::GetIdleTimeoutMs() const { return m_idleTimeoutMs; }
uint32_t ConnectionConfig::GetConnectTimeoutMs() const { return m_connectTimeoutMs; }

void ConnectionConfig::SetMaxPerService(uint32_t maxPerService) { m_maxPerService = maxPerService; }
void ConnectionConfig::SetIdleTimeoutMs(uint32_t idleTimeoutMs) { m_idleTimeoutMs = idleTimeoutMs; }
void ConnectionConfig::SetConnectTimeoutMs(uint32_t connectTimeoutMs) { m_connectTimeoutMs = connectTimeoutMs; }