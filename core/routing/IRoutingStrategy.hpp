#ifndef BIT_BRIDGE_IROUTING_STRATEGY_HPP
#define BIT_BRIDGE_IROUTING_STRATEGY_HPP

#include <expected>
#include <string_view>
#include "RoutingError.hpp"

class ServicePool;

class IRoutingStrategy {
public:
    IRoutingStrategy() = default;

    IRoutingStrategy(const IRoutingStrategy &) = default;

    IRoutingStrategy(IRoutingStrategy &&) noexcept = default;

    virtual ~IRoutingStrategy() = default;

    IRoutingStrategy &operator=(const IRoutingStrategy &) = default;

    IRoutingStrategy &operator=(IRoutingStrategy &&) noexcept = default;

    [[nodiscard]] virtual std::expected<size_t, RoutingError> SelectService(
        const ServicePool &pool, std::string_view routingKey) = 0;
};

#endif //BIT_BRIDGE_IROUTING_STRATEGY_HPP