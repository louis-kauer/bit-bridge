#include "ServicePool.hpp"

ServicePool::ServicePool(const std::vector<ServiceNode> &services) {
    m_services.reserve(services.size());
    for (const auto &node: services) {
        m_services.emplace_back(node);
    }
}

size_t ServicePool::GetSize() const { return m_services.size(); }

ServiceState &ServicePool::GetService(size_t index) { return m_services.at(index); }

const ServiceState &ServicePool::GetService(size_t index) const { return m_services.at(index); }

size_t ServicePool::GetHealthyCount() const {
    size_t count = 0;
    for (const auto &service: m_services) {
        if (service.IsHealthy()) {
            ++count;
        }
    }
    return count;
}