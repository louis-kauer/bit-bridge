#ifndef BIT_BRIDGE_CONSISTENT_HASH_STRATEGY_HPP
#define BIT_BRIDGE_CONSISTENT_HASH_STRATEGY_HPP

#include "IRoutingStrategy.hpp"

#include <utility>
#include <vector>

class ServicePool;

class ConsistentHashStrategy : public IRoutingStrategy {
public:
    explicit ConsistentHashStrategy(const ServicePool &pool);

    ConsistentHashStrategy(const ConsistentHashStrategy &other) = default;

    ConsistentHashStrategy(ConsistentHashStrategy &&other) noexcept = default;

    ~ConsistentHashStrategy() override = default;

    ConsistentHashStrategy &operator=(const ConsistentHashStrategy &other) = default;

    ConsistentHashStrategy &operator=(ConsistentHashStrategy &&other) noexcept = default;

    [[nodiscard]] std::expected<size_t, RoutingError> SelectService(
        const ServicePool &pool, std::string_view routingKey) override;

private:
    static constexpr size_t VNODES_PER_SERVICE = 150;

    std::vector<std::pair<uint64_t, size_t> > m_ring;
};

#endif //BIT_BRIDGE_CONSISTENT_HASH_STRATEGY_HPP