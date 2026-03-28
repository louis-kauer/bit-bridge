#ifndef BIT_BRIDGE_LOAD_BALANCER_CONFIG_HPP
#define BIT_BRIDGE_LOAD_BALANCER_CONFIG_HPP

#include <string>
#include <vector>
#include "ServiceNode.hpp"
#include "HealthCheckConfig.hpp"
#include "ConnectionConfig.hpp"

class LoadBalancerConfig {
public:
    LoadBalancerConfig();

    LoadBalancerConfig(const LoadBalancerConfig &other) = default;

    LoadBalancerConfig(LoadBalancerConfig &&other) noexcept = default;

    ~LoadBalancerConfig() = default;

    LoadBalancerConfig &operator=(const LoadBalancerConfig &other) = default;

    LoadBalancerConfig &operator=(LoadBalancerConfig &&other) noexcept = default;

    [[nodiscard]] std::string GetName() const;

    [[nodiscard]] std::string GetListenAddress() const;

    [[nodiscard]] uint16_t GetListenPort() const;

    [[nodiscard]] std::string GetRoutingAlgorithm() const;

    void SetName(const std::string &name);

    void SetListenAddress(const std::string &address);

    void SetListenPort(uint16_t port);

    void SetRoutingAlgorithm(const std::string &algorithm);

    void AddService(const ServiceNode &service);

    void RemoveService(size_t index);

    [[nodiscard]] const std::vector<ServiceNode> &GetServices() const;

    [[nodiscard]] size_t GetServiceCount() const;

    [[nodiscard]] HealthCheckConfig &GetHealthCheck();

    [[nodiscard]] const HealthCheckConfig &GetHealthCheck() const;

    void SetHealthCheck(const HealthCheckConfig &healthCheck);

    [[nodiscard]] ConnectionConfig &GetConnection();

    [[nodiscard]] const ConnectionConfig &GetConnection() const;

    void SetConnection(const ConnectionConfig &connection);

private:
    std::string m_name;
    std::string m_listenAddress;
    uint16_t m_listenPort = 0;
    std::string m_routingAlgorithm;
    std::vector<ServiceNode> m_services;
    HealthCheckConfig m_healthCheck;
    ConnectionConfig m_connection;
};

#endif //BIT_BRIDGE_LOAD_BALANCER_CONFIG_HPP