#include <gtest/gtest.h>
#include "ConsistentHashStrategy.hpp"
#include "ServicePool.hpp"

#include <format>
#include <set>

class ConsistentHashStrategyTest : public ::testing::Test {
protected:
    [[nodiscard]] static std::vector<ServiceNode> MakeNodes(size_t count) {
        std::vector<ServiceNode> nodes;
        for (size_t i = 0; i < count; ++i) {
            nodes.emplace_back(
                std::format("svc-{}", i),
                std::format("10.0.0.{}", i + 1),
                static_cast<uint16_t>(8080 + i), 1);
        }
        return nodes;
    }
};

TEST_F(ConsistentHashStrategyTest, ReturnsErrorOnEmptyPool) {
    ServicePool pool;
    ConsistentHashStrategy strategy(pool);
    auto result = strategy.SelectService(pool, "10.0.0.1");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), RoutingError::PoolEmpty);
}

TEST_F(ConsistentHashStrategyTest, ReturnsErrorWhenAllUnhealthy) {
    auto nodes = MakeNodes(3);
    ServicePool pool(nodes);
    ConsistentHashStrategy strategy(pool);

    pool.GetService(0).SetHealthy(false);
    pool.GetService(1).SetHealthy(false);
    pool.GetService(2).SetHealthy(false);

    auto result = strategy.SelectService(pool, "10.0.0.1");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), RoutingError::NoHealthyServices);
}

TEST_F(ConsistentHashStrategyTest, SameKeyAlwaysMapsToSameService) {
    auto nodes = MakeNodes(5);
    ServicePool pool(nodes);
    ConsistentHashStrategy strategy(pool);

    auto first = strategy.SelectService(pool, "client-1");
    ASSERT_TRUE(first.has_value());

    for (int i = 0; i < 100; ++i) {
        auto result = strategy.SelectService(pool, "client-1");
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, *first);
    }
}

TEST_F(ConsistentHashStrategyTest, DifferentKeysCanMapToDifferentServices) {
    auto nodes = MakeNodes(10);
    ServicePool pool(nodes);
    ConsistentHashStrategy strategy(pool);

    std::set<size_t> selectedServices;
    for (int i = 0; i < 1000; ++i) {
        auto result = strategy.SelectService(pool, std::format("client-{}", i));
        ASSERT_TRUE(result.has_value());
        selectedServices.insert(*result);
    }

    EXPECT_GT(selectedServices.size(), 1);
}

TEST_F(ConsistentHashStrategyTest, GhostNodesPreserveMapping) {
    auto nodes = MakeNodes(3);
    ServicePool pool(nodes);
    ConsistentHashStrategy strategy(pool);

    auto beforeResult = strategy.SelectService(pool, "client-1");
    ASSERT_TRUE(beforeResult.has_value());
    size_t beforeService = *beforeResult;

    size_t unhealthyIdx = (beforeService + 1) % 3;
    pool.GetService(unhealthyIdx).SetHealthy(false);

    auto afterResult = strategy.SelectService(pool, "client-1");
    ASSERT_TRUE(afterResult.has_value());
    EXPECT_EQ(*afterResult, beforeService);
}

TEST_F(ConsistentHashStrategyTest, SkipsUnhealthyAndFindsNext) {
    auto nodes = MakeNodes(3);
    ServicePool pool(nodes);
    ConsistentHashStrategy strategy(pool);

    auto result = strategy.SelectService(pool, "test-key");
    ASSERT_TRUE(result.has_value());
    size_t originalService = *result;

    pool.GetService(originalService).SetHealthy(false);

    auto newResult = strategy.SelectService(pool, "test-key");
    ASSERT_TRUE(newResult.has_value());
    EXPECT_NE(*newResult, originalService);
    EXPECT_TRUE(pool.GetService(*newResult).IsHealthy());
}

TEST_F(ConsistentHashStrategyTest, SingleServiceAlwaysSelected) {
    auto nodes = MakeNodes(1);
    ServicePool pool(nodes);
    ConsistentHashStrategy strategy(pool);

    for (int i = 0; i < 50; ++i) {
        auto result = strategy.SelectService(pool, std::format("key-{}", i));
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(*result, 0);
    }
}