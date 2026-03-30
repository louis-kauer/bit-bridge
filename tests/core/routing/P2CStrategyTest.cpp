#include <gtest/gtest.h>
#include "P2CStrategy.hpp"
#include "ServicePool.hpp"

TEST(P2CStrategyTest, ReturnsErrorOnEmptyPool) {
    P2CStrategy strategy(42);
    ServicePool pool;
    auto result = strategy.SelectService(pool, "10.0.0.1");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), RoutingError::PoolEmpty);
}

TEST(P2CStrategyTest, ReturnsErrorWhenAllUnhealthy) {
    std::vector<ServiceNode> nodes = {
        ServiceNode("a", "1.1.1.1", 80, 1),
        ServiceNode("b", "2.2.2.2", 80, 1),
    };
    ServicePool pool(nodes);
    pool.GetService(0).SetHealthy(false);
    pool.GetService(1).SetHealthy(false);

    P2CStrategy strategy(42);
    auto result = strategy.SelectService(pool, "10.0.0.1");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), RoutingError::NoHealthyServices);
}

TEST(P2CStrategyTest, ReturnsSingleHealthyService) {
    std::vector<ServiceNode> nodes = {
        ServiceNode("a", "1.1.1.1", 80, 1),
        ServiceNode("b", "2.2.2.2", 80, 1),
        ServiceNode("c", "3.3.3.3", 80, 1),
    };
    ServicePool pool(nodes);
    pool.GetService(0).SetHealthy(false);
    pool.GetService(2).SetHealthy(false);

    P2CStrategy strategy(42);
    for (int i = 0; i < 100; ++i) {
        auto result = strategy.SelectService(pool, "10.0.0.1");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, 1);
    }
}

TEST(P2CStrategyTest, PrefersLighterBackend) {
    std::vector<ServiceNode> nodes = {
        ServiceNode("heavy", "1.1.1.1", 80, 1),
        ServiceNode("light", "2.2.2.2", 80, 1),
    };
    ServicePool pool(nodes);

    // Make "heavy" have 100 connections, "light" has 0
    for (int i = 0; i < 100; ++i) {
        pool.GetService(0).IncrementConnections();
    }

    P2CStrategy strategy(42);
    size_t lightCount = 0;
    for (int i = 0; i < 1000; ++i) {
        auto result = strategy.SelectService(pool, "10.0.0.1");
        ASSERT_TRUE(result.has_value());
        if (*result == 1) {
            ++lightCount;
        }
    }

    // Light backend should be picked overwhelmingly
    EXPECT_GT(lightCount, 900);
}

TEST(P2CStrategyTest, NeverSelectsUnhealthyService) {
    std::vector<ServiceNode> nodes = {
        ServiceNode("healthy-a", "1.1.1.1", 80, 1),
        ServiceNode("unhealthy", "2.2.2.2", 80, 1),
        ServiceNode("healthy-b", "3.3.3.3", 80, 1),
    };
    ServicePool pool(nodes);
    pool.GetService(1).SetHealthy(false);

    P2CStrategy strategy(42);
    for (int i = 0; i < 1000; ++i) {
        auto result = strategy.SelectService(pool, "10.0.0.1");
        ASSERT_TRUE(result.has_value());
        EXPECT_NE(*result, 1);
    }
}

TEST(P2CStrategyTest, DistributesAcrossEqualServices) {
    std::vector<ServiceNode> nodes = {
        ServiceNode("a", "1.1.1.1", 80, 1),
        ServiceNode("b", "2.2.2.2", 80, 1),
        ServiceNode("c", "3.3.3.3", 80, 1),
    };
    ServicePool pool(nodes);

    P2CStrategy strategy(42);
    std::array<size_t, 3> counts{};
    for (int i = 0; i < 3000; ++i) {
        auto result = strategy.SelectService(pool, "10.0.0.1");
        ASSERT_TRUE(result.has_value());
        counts[*result]++;
    }

    // Each should be picked at least once
    for (size_t count: counts) {
        EXPECT_GT(count, 0);
    }
}