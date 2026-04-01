#include "LoadBalancerConfig.hpp"

LoadBalancerConfig::LoadBalancerConfig()
    : m_name("bit-bridge-cluster")
      , m_listenAddress("0.0.0.0")
      , m_listenPort(8080)
      , m_routingAlgorithm("p2c")
      , m_services()
      , m_healthCheck()
      , m_connection() {
}

std::string LoadBalancerConfig::GetName() const { return m_name; }
std::string LoadBalancerConfig::GetListenAddress() const { return m_listenAddress; }
uint16_t LoadBalancerConfig::GetListenPort() const { return m_listenPort; }
std::string LoadBalancerConfig::GetRoutingAlgorithm() const { return m_routingAlgorithm; }

void LoadBalancerConfig::SetName(const std::string &name) { m_name = name; }
void LoadBalancerConfig::SetListenAddress(const std::string &address) { m_listenAddress = address; }
void LoadBalancerConfig::SetListenPort(uint16_t port) { m_listenPort = port; }
void LoadBalancerConfig::SetRoutingAlgorithm(const std::string &algorithm) { m_routingAlgorithm = algorithm; }

void LoadBalancerConfig::AddService(const ServiceNode &service) {
    m_services.push_back(service);
}

void LoadBalancerConfig::ClearServices() {
    m_services.clear();
}

void LoadBalancerConfig::RemoveService(size_t index) {
    if (index < m_services.size()) {
        m_services.erase(m_services.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

const std::vector<ServiceNode> &LoadBalancerConfig::GetServices() const { return m_services; }
size_t LoadBalancerConfig::GetServiceCount() const { return m_services.size(); }

HealthCheckConfig &LoadBalancerConfig::GetHealthCheck() { return m_healthCheck; }
const HealthCheckConfig &LoadBalancerConfig::GetHealthCheck() const { return m_healthCheck; }
void LoadBalancerConfig::SetHealthCheck(const HealthCheckConfig &healthCheck) { m_healthCheck = healthCheck; }

ConnectionConfig &LoadBalancerConfig::GetConnection() { return m_connection; }
const ConnectionConfig &LoadBalancerConfig::GetConnection() const { return m_connection; }
void LoadBalancerConfig::SetConnection(const ConnectionConfig &connection) { m_connection = connection; }