#include <gtest/gtest.h>
#include "HealthCheckConfig.hpp"

TEST(HealthCheckConfigTest, DefaultConstructorSetsDefaults) {
    HealthCheckConfig hc;
    EXPECT_TRUE(hc.GetEnabled());
    EXPECT_EQ(hc.GetIntervalMs(), 5000);
    EXPECT_EQ(hc.GetTimeoutMs(), 2000);
    EXPECT_EQ(hc.GetUnhealthyThreshold(), 3);
}

TEST(HealthCheckConfigTest, ParameterizedConstructorSetsValues) {
    HealthCheckConfig hc(false, 10000, 3000, 5);
    EXPECT_FALSE(hc.GetEnabled());
    EXPECT_EQ(hc.GetIntervalMs(), 10000);
    EXPECT_EQ(hc.GetTimeoutMs(), 3000);
    EXPECT_EQ(hc.GetUnhealthyThreshold(), 5);
}

TEST(HealthCheckConfigTest, CopyConstructorCopiesAllFields) {
    HealthCheckConfig original(false, 1000, 500, 10);
    HealthCheckConfig copy(original);
    EXPECT_EQ(copy.GetEnabled(), original.GetEnabled());
    EXPECT_EQ(copy.GetIntervalMs(), original.GetIntervalMs());
    EXPECT_EQ(copy.GetTimeoutMs(), original.GetTimeoutMs());
    EXPECT_EQ(copy.GetUnhealthyThreshold(), original.GetUnhealthyThreshold());
}

TEST(HealthCheckConfigTest, SettersUpdateValues) {
    HealthCheckConfig hc;
    hc.SetEnabled(false);
    hc.SetIntervalMs(7000);
    hc.SetTimeoutMs(4000);
    hc.SetUnhealthyThreshold(8);
    EXPECT_FALSE(hc.GetEnabled());
    EXPECT_EQ(hc.GetIntervalMs(), 7000);
    EXPECT_EQ(hc.GetTimeoutMs(), 4000);
    EXPECT_EQ(hc.GetUnhealthyThreshold(), 8);
}