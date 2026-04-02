#include <gtest/gtest.h>
#include "LoadBalancerConfig.hpp"

TEST(LoadBalancerConfigTest, DefaultConstructorSetsDefaults) {
    LoadBalancerConfig config;
    EXPECT_EQ(config.GetName(), "my-bit-bridge-lb");
    EXPECT_EQ(config.GetListenAddress(), "0.0.0.0");
    EXPECT_EQ(config.GetListenPort(), 8080);
    EXPECT_EQ(config.GetRoutingAlgorithm(), "p2c");
    EXPECT_EQ(config.GetServiceCount(), 0);
}

TEST(LoadBalancerConfigTest, SettersUpdateValues) {
    LoadBalancerConfig config;
    config.SetName("test-cluster");
    config.SetListenAddress("127.0.0.1");
    config.SetListenPort(9090);
    config.SetRoutingAlgorithm("round-robin");
    EXPECT_EQ(config.GetName(), "test-cluster");
    EXPECT_EQ(config.GetListenAddress(), "127.0.0.1");
    EXPECT_EQ(config.GetListenPort(), 9090);
    EXPECT_EQ(config.GetRoutingAlgorithm(), "round-robin");
}

TEST(LoadBalancerConfigTest, AddServiceIncreasesCount) {
    LoadBalancerConfig config;
    config.AddService(ServiceNode("web-1", "10.0.0.1", 80, 1));
    EXPECT_EQ(config.GetServiceCount(), 1);
    config.AddService(ServiceNode("web-2", "10.0.0.2", 80, 1));
    EXPECT_EQ(config.GetServiceCount(), 2);
}

TEST(LoadBalancerConfigTest, RemoveServiceDecreasesCount) {
    LoadBalancerConfig config;
    config.AddService(ServiceNode("web-1", "10.0.0.1", 80, 1));
    config.AddService(ServiceNode("web-2", "10.0.0.2", 80, 1));
    config.RemoveService(0);
    EXPECT_EQ(config.GetServiceCount(), 1);
    EXPECT_EQ(config.GetServices()[0].GetName(), "web-2");
}

TEST(LoadBalancerConfigTest, RemoveServiceWithInvalidIndexDoesNothing) {
    LoadBalancerConfig config;
    config.AddService(ServiceNode("web-1", "10.0.0.1", 80, 1));
    config.RemoveService(5);
    EXPECT_EQ(config.GetServiceCount(), 1);
}

TEST(LoadBalancerConfigTest, RemoveFromEmptyConfigDoesNothing) {
    LoadBalancerConfig config;
    config.RemoveService(0);
    EXPECT_EQ(config.GetServiceCount(), 0);
}

TEST(LoadBalancerConfigTest, GetServicesReturnsCorrectData) {
    LoadBalancerConfig config;
    config.AddService(ServiceNode("api-1", "172.16.0.1", 3000, 2));
    const auto &services = config.GetServices();
    EXPECT_EQ(services.size(), 1);
    EXPECT_EQ(services[0].GetName(), "api-1");
    EXPECT_EQ(services[0].GetIp(), "172.16.0.1");
    EXPECT_EQ(services[0].GetPort(), 3000);
    EXPECT_EQ(services[0].GetWeight(), 2);
}

TEST(LoadBalancerConfigTest, CopyConstructorCopiesEverything) {
    LoadBalancerConfig original;
    original.SetName("copy-test");
    original.AddService(ServiceNode("svc", "1.2.3.4", 8080, 1));
    original.SetHealthCheck(HealthCheckConfig(false, 1000, 500, 10));
    original.SetConnection(ConnectionConfig(256, 15000, 3000));

    LoadBalancerConfig copy(original);
    EXPECT_EQ(copy.GetName(), "copy-test");
    EXPECT_EQ(copy.GetServiceCount(), 1);
    EXPECT_FALSE(copy.GetHealthCheck().GetEnabled());
    EXPECT_EQ(copy.GetConnection().GetMaxPerService(), 256);
}

TEST(LoadBalancerConfigTest, HealthCheckDefaultsAreCorrect) {
    LoadBalancerConfig config;
    const auto &hc = config.GetHealthCheck();
    EXPECT_TRUE(hc.GetEnabled());
    EXPECT_EQ(hc.GetIntervalMs(), 5000);
    EXPECT_EQ(hc.GetTimeoutMs(), 2000);
    EXPECT_EQ(hc.GetUnhealthyThreshold(), 3);
}

TEST(LoadBalancerConfigTest, ConnectionDefaultsAreCorrect) {
    LoadBalancerConfig config;
    const auto &conn = config.GetConnection();
    EXPECT_EQ(conn.GetMaxPerService(), 1024);
    EXPECT_EQ(conn.GetIdleTimeoutMs(), 30000);
    EXPECT_EQ(conn.GetConnectTimeoutMs(), 5000);
}