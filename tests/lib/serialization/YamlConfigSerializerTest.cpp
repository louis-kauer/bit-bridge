#include <gtest/gtest.h>
#include "YamlConfigSerializer.hpp"
#include "LoadBalancerConfig.hpp"

#include <filesystem>
#include <fstream>

class YamlConfigSerializerTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_testDir = "test_output";
        std::filesystem::create_directories(m_testDir);
        m_testFile = m_testDir + "/test-config.yaml";
    }

    void TearDown() override {
        std::filesystem::remove_all(m_testDir);
    }

    std::string m_testDir;
    std::string m_testFile;
};

TEST_F(YamlConfigSerializerTest, SaveAndLoadRoundTrip) {
    LoadBalancerConfig original;
    original.SetName("test-cluster");
    original.SetListenAddress("127.0.0.1");
    original.SetListenPort(9090);
    original.SetRoutingAlgorithm("round-robin");
    original.AddService(ServiceNode("web-1", "10.0.0.1", 80, 2));
    original.AddService(ServiceNode("web-2", "10.0.0.2", 443, 3));
    original.SetHealthCheck(HealthCheckConfig(false, 10000, 3000, 5));
    original.SetConnection(ConnectionConfig(512, 60000, 10000));

    YamlConfigSerializer serializer;
    ASSERT_TRUE(serializer.Save(original, m_testFile));

    LoadBalancerConfig loaded;
    ASSERT_TRUE(serializer.Load(loaded, m_testFile));

    EXPECT_EQ(loaded.GetName(), "test-cluster");
    EXPECT_EQ(loaded.GetListenAddress(), "127.0.0.1");
    EXPECT_EQ(loaded.GetListenPort(), 9090);
    EXPECT_EQ(loaded.GetRoutingAlgorithm(), "round-robin");
    EXPECT_EQ(loaded.GetServiceCount(), 2);
    EXPECT_EQ(loaded.GetServices()[0].GetName(), "web-1");
    EXPECT_EQ(loaded.GetServices()[0].GetPort(), 80);
    EXPECT_EQ(loaded.GetServices()[0].GetWeight(), 2);
    EXPECT_EQ(loaded.GetServices()[1].GetName(), "web-2");
    EXPECT_EQ(loaded.GetServices()[1].GetPort(), 443);
    EXPECT_FALSE(loaded.GetHealthCheck().GetEnabled());
    EXPECT_EQ(loaded.GetHealthCheck().GetIntervalMs(), 10000);
    EXPECT_EQ(loaded.GetConnection().GetMaxPerService(), 512);
    EXPECT_EQ(loaded.GetConnection().GetConnectTimeoutMs(), 10000);
}

TEST_F(YamlConfigSerializerTest, SaveEmptyServicesList) {
    LoadBalancerConfig config;
    YamlConfigSerializer serializer;
    ASSERT_TRUE(serializer.Save(config, m_testFile));

    LoadBalancerConfig loaded;
    ASSERT_TRUE(serializer.Load(loaded, m_testFile));
    EXPECT_EQ(loaded.GetServiceCount(), 0);
}

TEST_F(YamlConfigSerializerTest, LoadFromNonexistentFileReturnsFalse) {
    YamlConfigSerializer serializer;
    LoadBalancerConfig config;
    EXPECT_FALSE(serializer.Load(config, "nonexistent/path/config.yaml"));
}

TEST_F(YamlConfigSerializerTest, LoadFromMalformedYamlReturnsFalse) {
    std::ofstream fout(m_testFile);
    fout << "{{{{invalid yaml content::::";
    fout.close();

    YamlConfigSerializer serializer;
    LoadBalancerConfig config;
    EXPECT_FALSE(serializer.Load(config, m_testFile));
}

TEST_F(YamlConfigSerializerTest, LoadFromEmptyFileUsesDefaults) {
    std::ofstream fout(m_testFile);
    fout.close();

    YamlConfigSerializer serializer;
    LoadBalancerConfig config;
    EXPECT_TRUE(serializer.Load(config, m_testFile));
    EXPECT_EQ(config.GetServiceCount(), 0);
}

TEST_F(YamlConfigSerializerTest, SaveCreatesParentDirectories) {
    LoadBalancerConfig config;
    YamlConfigSerializer serializer;
    std::string nestedPath = m_testDir + "/nested/deep/config.yaml";
    ASSERT_TRUE(serializer.Save(config, nestedPath));
    EXPECT_TRUE(std::filesystem::exists(nestedPath));
}

TEST_F(YamlConfigSerializerTest, LoadPartialYamlUsesDefaults) {
    std::ofstream fout(m_testFile);
    fout << "name: partial-config\nlistenPort: 3000\n";
    fout.close();

    YamlConfigSerializer serializer;
    LoadBalancerConfig config;
    ASSERT_TRUE(serializer.Load(config, m_testFile));
    EXPECT_EQ(config.GetName(), "partial-config");
    EXPECT_EQ(config.GetListenPort(), 3000);
    EXPECT_EQ(config.GetServiceCount(), 0);
}