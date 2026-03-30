#ifndef BIT_BRIDGE_SERVICE_POOL_HPP
#define BIT_BRIDGE_SERVICE_POOL_HPP

#include <vector>
#include "ServiceState.hpp"
#include "ServiceNode.hpp"

class ServicePool {
public:
    ServicePool() = default;

    explicit ServicePool(const std::vector<ServiceNode> &services);

    ServicePool(const ServicePool &other) = default;

    ServicePool(ServicePool &&other) noexcept = default;

    ~ServicePool() = default;

    ServicePool &operator=(const ServicePool &other) = default;

    ServicePool &operator=(ServicePool &&other) noexcept = default;

    [[nodiscard]] size_t GetSize() const;

    [[nodiscard]] ServiceState &GetService(size_t index);

    [[nodiscard]] const ServiceState &GetService(size_t index) const;

    [[nodiscard]] size_t GetHealthyCount() const;

private:
    std::vector<ServiceState> m_services;
};

#endif //BIT_BRIDGE_SERVICE_POOL_HPP