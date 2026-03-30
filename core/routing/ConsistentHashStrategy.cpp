#include "ConsistentHashStrategy.hpp"
#include "ServicePool.hpp"
#include "FnvHash.hpp"
#include "HashCollisionError.hpp"

#include <algorithm>
#include <format>
#include <print>
#include <ranges>
#include <string>

ConsistentHashStrategy::ConsistentHashStrategy(const ServicePool &pool) {
    m_ring.reserve(pool.GetSize() * VNODES_PER_SERVICE);

    for (size_t i = 0; i < pool.GetSize(); ++i) {
        const auto &name = pool.GetService(i).GetNode().GetName();
        for (size_t v = 0; v < VNODES_PER_SERVICE; ++v) {
            m_ring.emplace_back(fnv::Hash64(std::format("{}#{}", name, v)), i);
        }
    }

    std::ranges::sort(m_ring, {}, &std::pair<uint64_t, size_t>::first);

    // Runtime collision check — fail loudly in all builds
    for (size_t i = 1; i < m_ring.size(); ++i) {
        if (m_ring.at(i).first == m_ring.at(i - 1).first) {
            std::println(stderr, "Fatal: hash collision at vnode {} — ring integrity compromised", i);
            throw HashCollisionError(i);
        }
    }
}

std::expected<size_t, RoutingError> ConsistentHashStrategy::SelectService(
    const ServicePool &pool, const std::string_view routingKey) {
    if (pool.GetSize() == 0 || m_ring.empty()) {
        return std::unexpected(RoutingError::PoolEmpty);
    }

    const uint64_t hash = fnv::Hash64(routingKey);

    const auto it = std::ranges::upper_bound(
        m_ring, hash, {}, &std::pair<uint64_t, size_t>::first);

    const auto startIdx = static_cast<size_t>(std::distance(m_ring.begin(), it));
    const size_t ringSize = m_ring.size();

    for (size_t walked = 0; walked < ringSize; ++walked) {
        size_t idx = (startIdx + walked) % ringSize;
        if (pool.GetService(m_ring.at(idx).second).IsHealthy()) {
            return m_ring.at(idx).second;
        }
    }

    return std::unexpected(RoutingError::NoHealthyServices);
}
