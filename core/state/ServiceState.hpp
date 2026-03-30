#ifndef BIT_BRIDGE_SERVICE_STATE_HPP
#define BIT_BRIDGE_SERVICE_STATE_HPP

#include <atomic>
#include <cstddef>
#include "ServiceNode.hpp"

class ServiceState {
public:
    explicit ServiceState(const ServiceNode &node);

    ServiceState(const ServiceState &other);

    ServiceState(ServiceState &&other) noexcept;

    ~ServiceState() = default;

    ServiceState &operator=(const ServiceState &other);

    ServiceState &operator=(ServiceState &&other) noexcept;

    [[nodiscard]] const ServiceNode &GetNode() const;

    [[nodiscard]] size_t GetActiveConnections() const;

    [[nodiscard]] bool IsHealthy() const;

    [[nodiscard]] size_t GetConsecutiveFailures() const;

    void IncrementConnections();

    void DecrementConnections();

    void SetHealthy(bool healthy);

    void IncrementFailures();

    void ResetFailures();

private:
    ServiceNode m_node;
    std::atomic<size_t> m_activeConnections{0};
    std::atomic<bool> m_healthy{true};
    std::atomic<size_t> m_consecutiveFailures{0};
};

#endif //BIT_BRIDGE_SERVICE_STATE_HPP