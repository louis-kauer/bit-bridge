#include <gtest/gtest.h>
#include "ConnectionConfig.hpp"

TEST(ConnectionConfigTest, DefaultConstructorSetsDefaults) {
    ConnectionConfig conn;
    EXPECT_EQ(conn.GetMaxPerService(), 1024);
    EXPECT_EQ(conn.GetIdleTimeoutMs(), 30000);
    EXPECT_EQ(conn.GetConnectTimeoutMs(), 5000);
}

TEST(ConnectionConfigTest, ParameterizedConstructorSetsValues) {
    ConnectionConfig conn(512, 60000, 10000);
    EXPECT_EQ(conn.GetMaxPerService(), 512);
    EXPECT_EQ(conn.GetIdleTimeoutMs(), 60000);
    EXPECT_EQ(conn.GetConnectTimeoutMs(), 10000);
}

TEST(ConnectionConfigTest, CopyConstructorCopiesAllFields) {
    ConnectionConfig original(256, 15000, 3000);
    ConnectionConfig copy(original);
    EXPECT_EQ(copy.GetMaxPerService(), original.GetMaxPerService());
    EXPECT_EQ(copy.GetIdleTimeoutMs(), original.GetIdleTimeoutMs());
    EXPECT_EQ(copy.GetConnectTimeoutMs(), original.GetConnectTimeoutMs());
}

TEST(ConnectionConfigTest, SettersUpdateValues) {
    ConnectionConfig conn;
    conn.SetMaxPerService(2048);
    conn.SetIdleTimeoutMs(45000);
    conn.SetConnectTimeoutMs(8000);
    EXPECT_EQ(conn.GetMaxPerService(), 2048);
    EXPECT_EQ(conn.GetIdleTimeoutMs(), 45000);
    EXPECT_EQ(conn.GetConnectTimeoutMs(), 8000);
}