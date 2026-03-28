#ifndef BIT_BRIDGE_SERVICE_NODE_HPP
#define BIT_BRIDGE_SERVICE_NODE_HPP

#include <string>
#include <cstdint>

class ServiceNode {
public:
    ServiceNode();

    ServiceNode(const std::string &name, const std::string &ip, uint16_t port, uint16_t weight);

    ServiceNode(const ServiceNode &other) = default;

    ServiceNode(ServiceNode &&other) noexcept = default;

    ~ServiceNode() = default;

    ServiceNode &operator=(const ServiceNode &other) = default;

    ServiceNode &operator=(ServiceNode &&other) noexcept = default;

    [[nodiscard]] std::string GetName() const;

    [[nodiscard]] std::string GetIp() const;

    [[nodiscard]] uint16_t GetPort() const;

    [[nodiscard]] uint16_t GetWeight() const;

    void SetName(const std::string &name);

    void SetIp(const std::string &ip);

    void SetPort(uint16_t port);

    void SetWeight(uint16_t weight);

    [[nodiscard]] bool IsValid() const;

private:
    static bool IsValidIpv4(const std::string &ip);

    std::string m_name;
    std::string m_ip;
    uint16_t m_port = 0;
    uint16_t m_weight = 1;
};

#endif //BIT_BRIDGE_SERVICE_NODE_HPP