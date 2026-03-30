#include <gtest/gtest.h>
#include "ServiceState.hpp"

TEST(ServiceStateTest, ConstructsWithNodeAndDefaults) {
    ServiceNode node("web-1", "10.0.0.1", 80, 1);
    ServiceState state(node);
    EXPECT_EQ(state.GetNode().GetName(), "web-1");
    EXPECT_EQ(state.GetActiveConnections(), 0);
    EXPECT_TRUE(state.IsHealthy());
    EXPECT_EQ(state.GetConsecutiveFailures(), 0);
}

TEST(ServiceStateTest, IncrementDecrementConnections) {
    ServiceState state(ServiceNode("s", "1.2.3.4", 80, 1));
    state.IncrementConnections();
    state.IncrementConnections();
    state.IncrementConnections();
    EXPECT_EQ(state.GetActiveConnections(), 3);
    state.DecrementConnections();
    EXPECT_EQ(state.GetActiveConnections(), 2);
}

TEST(ServiceStateTest, HealthTransitions) {
    ServiceState state(ServiceNode("s", "1.2.3.4", 80, 1));
    EXPECT_TRUE(state.IsHealthy());
    state.SetHealthy(false);
    EXPECT_FALSE(state.IsHealthy());
    state.SetHealthy(true);
    EXPECT_TRUE(state.IsHealthy());
}

TEST(ServiceStateTest, FailureCountingAndReset) {
    ServiceState state(ServiceNode("s", "1.2.3.4", 80, 1));
    state.IncrementFailures();
    state.IncrementFailures();
    state.IncrementFailures();
    EXPECT_EQ(state.GetConsecutiveFailures(), 3);
    state.ResetFailures();
    EXPECT_EQ(state.GetConsecutiveFailures(), 0);
}

TEST(ServiceStateTest, CopyConstructorCopiesAtomics) {
    ServiceState original(ServiceNode("s", "1.2.3.4", 80, 1));
    original.IncrementConnections();
    original.IncrementConnections();
    original.SetHealthy(false);
    original.IncrementFailures();

    ServiceState copy(original);
    EXPECT_EQ(copy.GetActiveConnections(), 2);
    EXPECT_FALSE(copy.IsHealthy());
    EXPECT_EQ(copy.GetConsecutiveFailures(), 1);
    EXPECT_EQ(copy.GetNode().GetName(), "s");
}

TEST(ServiceStateTest, MoveConstructorTransfersState) {
    ServiceState original(ServiceNode("s", "1.2.3.4", 80, 1));
    original.IncrementConnections();
    original.SetHealthy(false);

    ServiceState moved(std::move(original));
    EXPECT_EQ(moved.GetActiveConnections(), 1);
    EXPECT_FALSE(moved.IsHealthy());
}