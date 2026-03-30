#include "ServiceState.hpp"

ServiceState::ServiceState(const ServiceNode &node)
    : m_node(node) {
}

ServiceState::ServiceState(const ServiceState &other)
    : m_node(other.m_node)
      , m_activeConnections(other.m_activeConnections.load(std::memory_order_relaxed))
      , m_healthy(other.m_healthy.load(std::memory_order_relaxed))
      , m_consecutiveFailures(other.m_consecutiveFailures.load(std::memory_order_relaxed)) {
}

ServiceState::ServiceState(ServiceState &&other) noexcept
    : m_node(std::move(other.m_node))
      , m_activeConnections(other.m_activeConnections.load(std::memory_order_relaxed))
      , m_healthy(other.m_healthy.load(std::memory_order_relaxed))
      , m_consecutiveFailures(other.m_consecutiveFailures.load(std::memory_order_relaxed)) {
}

ServiceState &ServiceState::operator=(const ServiceState &other) {
    if (this != &other) {
        m_node = other.m_node;
        m_activeConnections.store(other.m_activeConnections.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_healthy.store(other.m_healthy.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_consecutiveFailures.store(other.m_consecutiveFailures.load(std::memory_order_relaxed),
                                    std::memory_order_relaxed);
    }
    return *this;
}

ServiceState &ServiceState::operator=(ServiceState &&other) noexcept {
    if (this != &other) {
        m_node = std::move(other.m_node);
        m_activeConnections.store(other.m_activeConnections.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_healthy.store(other.m_healthy.load(std::memory_order_relaxed), std::memory_order_relaxed);
        m_consecutiveFailures.store(other.m_consecutiveFailures.load(std::memory_order_relaxed),
                                    std::memory_order_relaxed);
    }
    return *this;
}

const ServiceNode &ServiceState::GetNode() const { return m_node; }

size_t ServiceState::GetActiveConnections() const {
    return m_activeConnections.load(std::memory_order_relaxed);
}

bool ServiceState::IsHealthy() const {
    return m_healthy.load(std::memory_order_relaxed);
}

size_t ServiceState::GetConsecutiveFailures() const {
    return m_consecutiveFailures.load(std::memory_order_relaxed);
}

void ServiceState::IncrementConnections() {
    m_activeConnections.fetch_add(1, std::memory_order_relaxed);
}

void ServiceState::DecrementConnections() {
    m_activeConnections.fetch_sub(1, std::memory_order_relaxed);
}

void ServiceState::SetHealthy(bool healthy) {
    m_healthy.store(healthy, std::memory_order_relaxed);
}

void ServiceState::IncrementFailures() {
    m_consecutiveFailures.fetch_add(1, std::memory_order_relaxed);
}

void ServiceState::ResetFailures() {
    m_consecutiveFailures.store(0, std::memory_order_relaxed);
}