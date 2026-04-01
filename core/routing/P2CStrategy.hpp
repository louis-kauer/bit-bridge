#ifndef BIT_BRIDGE_P2C_STRATEGY_HPP
#define BIT_BRIDGE_P2C_STRATEGY_HPP

#include "IRoutingStrategy.hpp"
#include "Xoshiro256.hpp"

class P2CStrategy : public IRoutingStrategy {
public:
    P2CStrategy();

    explicit P2CStrategy(uint64_t seed);

    P2CStrategy(const P2CStrategy &other);

    P2CStrategy(P2CStrategy &&other) noexcept;

    ~P2CStrategy() override = default;

    P2CStrategy &operator=(const P2CStrategy &other);

    P2CStrategy &operator=(P2CStrategy &&other) noexcept;

    [[nodiscard]] std::expected<size_t, RoutingError> SelectService(
        const ServicePool &pool, std::string_view routingKey) override;

private:
    Xoshiro256StarStar m_rng;
};

#endif //BIT_BRIDGE_P2C_STRATEGY_HPP