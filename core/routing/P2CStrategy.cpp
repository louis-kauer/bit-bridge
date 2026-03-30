#include "P2CStrategy.hpp"
#include "ServicePool.hpp"

#include <random>
#include <vector>

P2CStrategy::P2CStrategy()
    : m_rng(std::random_device{}()) {
}

P2CStrategy::P2CStrategy(const uint64_t seed)
    : m_rng(seed) {
}

P2CStrategy::P2CStrategy(const P2CStrategy &other)
    : IRoutingStrategy(other)
      , m_rng(std::random_device{}()) {
}

P2CStrategy::P2CStrategy(P2CStrategy &&other) noexcept
    : IRoutingStrategy(std::move(other))
      , m_rng(std::move(other.m_rng)) {
}

P2CStrategy &P2CStrategy::operator=(const P2CStrategy &other) {
    if (this != &other) {
        IRoutingStrategy::operator=(other);
        m_rng = Xoshiro256StarStar(std::random_device{}());
    }
    return *this;
}

P2CStrategy &P2CStrategy::operator=(P2CStrategy &&other) noexcept {
    if (this != &other) {
        IRoutingStrategy::operator=(std::move(other));
        m_rng = std::move(other.m_rng);
    }
    return *this;
}

std::expected<size_t, RoutingError> P2CStrategy::SelectService(
    const ServicePool &pool, [[maybe_unused]] std::string_view routingKey) {
    if (pool.GetSize() == 0) {
        return std::unexpected(RoutingError::PoolEmpty);
    }

    std::vector<size_t> healthy;
    healthy.reserve(pool.GetSize());
    for (size_t i = 0; i < pool.GetSize(); ++i) {
        if (pool.GetService(i).IsHealthy()) {
            healthy.push_back(i);
        }
    }

    if (healthy.empty()) {
        return std::unexpected(RoutingError::NoHealthyServices);
    }
    if (healthy.size() == 1) {
        return healthy.at(0);
    }

    std::uniform_int_distribution<size_t> dist(0, healthy.size() - 1);
    const size_t a = dist(m_rng);
    size_t b = dist(m_rng);
    while (b == a) { // NOLINT(bugprone-infinite-loop)
        b = dist(m_rng);
    }

    if (pool.GetService(healthy.at(a)).GetActiveConnections() <=
        pool.GetService(healthy.at(b)).GetActiveConnections()) {
        return healthy.at(a);
    }
    return healthy.at(b);
}