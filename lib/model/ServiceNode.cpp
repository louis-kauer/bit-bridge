#include "ServiceNode.hpp"

#include <sstream>
#include <utility>

ServiceNode::ServiceNode()
    : m_port(0)
      , m_weight(1) {
}

ServiceNode::ServiceNode(std::string name, std::string ip, uint16_t port, uint16_t weight)
    : m_name(std::move(name))
      , m_ip(std::move(ip))
      , m_port(port)
      , m_weight(weight) {
}

std::string ServiceNode::GetName() const { return m_name; }
std::string ServiceNode::GetIp() const { return m_ip; }
uint16_t ServiceNode::GetPort() const { return m_port; }
uint16_t ServiceNode::GetWeight() const { return m_weight; }

void ServiceNode::SetName(const std::string &name) { m_name = name; }
void ServiceNode::SetIp(const std::string &ip) { m_ip = ip; }
void ServiceNode::SetPort(uint16_t port) { m_port = port; }
void ServiceNode::SetWeight(uint16_t weight) { m_weight = weight; }

bool ServiceNode::IsValid() const {
    return !m_name.empty() && !m_ip.empty() && m_port != 0 && IsValidIpv4(m_ip);
}

bool ServiceNode::IsValidIpv4(const std::string &ip) {
    std::istringstream stream(ip);
    std::string token;
    size_t octetCount = 0;

    while (std::getline(stream, token, '.')) {
        if (token.empty() || token.size() > 3) {
            return false;
        }
        if (token.size() > 1 && token[0] == '0') {
            return false;
        }

        for (char c: token) {
            if (c < '0' || c > '9') {
                return false;
            }
        }

        if (std::stoi(token) > 255) {
            return false;
        }
        ++octetCount;
    }

    return octetCount == 4 && ip.back() != '.';
}