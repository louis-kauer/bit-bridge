#include <gtest/gtest.h>
#include "ServicePool.hpp"

TEST(ServicePoolTest, ConstructFromServices) {
    std::vector<ServiceNode> nodes = {
        ServiceNode("web-1", "10.0.0.1", 80, 1),
        ServiceNode("web-2", "10.0.0.2", 80, 1),
        ServiceNode("web-3", "10.0.0.3", 80, 1),
    };
    ServicePool pool(nodes);
    EXPECT_EQ(pool.GetSize(), 3);
}

TEST(ServicePoolTest, DefaultConstructorIsEmpty) {
    ServicePool pool;
    EXPECT_EQ(pool.GetSize(), 0);
}

TEST(ServicePoolTest, GetServiceReturnsCorrectNode) {
    std::vector<ServiceNode> nodes = {
        ServiceNode("api-1", "172.16.0.1", 3000, 2),
        ServiceNode("api-2", "172.16.0.2", 3001, 1),
    };
    ServicePool pool(nodes);
    EXPECT_EQ(pool.GetService(0).GetNode().GetName(), "api-1");
    EXPECT_EQ(pool.GetService(1).GetNode().GetPort(), 3001);
}

TEST(ServicePoolTest, GetServiceOutOfBoundsThrows) {
    ServicePool pool;
    EXPECT_THROW([[maybe_unused]] auto &svc = pool.GetService(0), std::out_of_range);
}

TEST(ServicePoolTest, GetHealthyCountReflectsState) {
    std::vector<ServiceNode> nodes = {
        ServiceNode("a", "1.1.1.1", 80, 1),
        ServiceNode("b", "2.2.2.2", 80, 1),
        ServiceNode("c", "3.3.3.3", 80, 1),
    };
    ServicePool pool(nodes);
    EXPECT_EQ(pool.GetHealthyCount(), 3);

    pool.GetService(1).SetHealthy(false);
    EXPECT_EQ(pool.GetHealthyCount(), 2);

    pool.GetService(0).SetHealthy(false);
    pool.GetService(2).SetHealthy(false);
    EXPECT_EQ(pool.GetHealthyCount(), 0);
}

TEST(ServicePoolTest, AllServicesStartHealthy) {
    std::vector<ServiceNode> nodes = {
        ServiceNode("a", "1.1.1.1", 80, 1),
        ServiceNode("b", "2.2.2.2", 80, 1),
    };
    ServicePool pool(nodes);
    for (size_t i = 0; i < pool.GetSize(); ++i) {
        EXPECT_TRUE(pool.GetService(i).IsHealthy());
        EXPECT_EQ(pool.GetService(i).GetActiveConnections(), 0);
    }
}