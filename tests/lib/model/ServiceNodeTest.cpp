#include <gtest/gtest.h>
#include "ServiceNode.hpp"

TEST(ServiceNodeTest, DefaultConstructorCreatesInvalidNode) {
    ServiceNode node;
    EXPECT_FALSE(node.IsValid());
    EXPECT_TRUE(node.GetName().empty());
    EXPECT_TRUE(node.GetIp().empty());
    EXPECT_EQ(node.GetPort(), 0);
    EXPECT_EQ(node.GetWeight(), 1);
}

TEST(ServiceNodeTest, ParameterizedConstructorSetsValues) {
    ServiceNode node("web-1", "192.168.1.10", 80, 2);
    EXPECT_EQ(node.GetName(), "web-1");
    EXPECT_EQ(node.GetIp(), "192.168.1.10");
    EXPECT_EQ(node.GetPort(), 80);
    EXPECT_EQ(node.GetWeight(), 2);
}

TEST(ServiceNodeTest, CopyConstructorCopiesAllFields) {
    ServiceNode original("web-1", "10.0.0.1", 443, 5);
    const ServiceNode copy = original;
    EXPECT_EQ(copy.GetName(), "web-1");
    EXPECT_EQ(copy.GetIp(), "10.0.0.1");
    EXPECT_EQ(copy.GetPort(), 443);
    EXPECT_EQ(copy.GetWeight(), 5);
}

TEST(ServiceNodeTest, SettersUpdateValues) {
    ServiceNode node;
    node.SetName("api-1");
    node.SetIp("172.16.0.1");
    node.SetPort(3000);
    node.SetWeight(3);
    EXPECT_EQ(node.GetName(), "api-1");
    EXPECT_EQ(node.GetIp(), "172.16.0.1");
    EXPECT_EQ(node.GetPort(), 3000);
    EXPECT_EQ(node.GetWeight(), 3);
}

TEST(ServiceNodeTest, ValidNodePassesValidation) {
    ServiceNode node("web-1", "192.168.1.10", 80, 1);
    EXPECT_TRUE(node.IsValid());
}

TEST(ServiceNodeTest, EmptyNameIsInvalid) {
    ServiceNode node("", "192.168.1.10", 80, 1);
    EXPECT_FALSE(node.IsValid());
}

TEST(ServiceNodeTest, EmptyIpIsInvalid) {
    ServiceNode node("web-1", "", 80, 1);
    EXPECT_FALSE(node.IsValid());
}

TEST(ServiceNodeTest, ZeroPortIsInvalid) {
    ServiceNode node("web-1", "192.168.1.10", 0, 1);
    EXPECT_FALSE(node.IsValid());
}

TEST(ServiceNodeTest, ValidIpAddresses) {
    EXPECT_TRUE(ServiceNode("n", "0.0.0.0", 1, 1).IsValid());
    EXPECT_TRUE(ServiceNode("n", "255.255.255.255", 1, 1).IsValid());
    EXPECT_TRUE(ServiceNode("n", "10.0.0.1", 1, 1).IsValid());
    EXPECT_TRUE(ServiceNode("n", "192.168.1.100", 1, 1).IsValid());
}

TEST(ServiceNodeTest, InvalidIpFormats) {
    EXPECT_FALSE(ServiceNode("n", "256.0.0.1", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", "1.2.3", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", "1.2.3.4.5", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", "abc.def.ghi.jkl", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", "192.168.1.", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", ".168.1.1", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", "192.168.01.1", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", "192.168.1.999", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", "", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", "just-text", 1, 1).IsValid());
    EXPECT_FALSE(ServiceNode("n", "192.168.1.1.1", 1, 1).IsValid());
}

TEST(ServiceNodeTest, MaxPortIsValid) {
    ServiceNode node("web-1", "10.0.0.1", 65535, 1);
    EXPECT_TRUE(node.IsValid());
}

TEST(ServiceNodeTest, PortOneIsValid) {
    ServiceNode node("web-1", "10.0.0.1", 1, 1);
    EXPECT_TRUE(node.IsValid());
}