#include "LoadBalancerConfig.hpp"
#include "YamlConfigSerializer.hpp"

#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: bit_bridge_lb <config-file>" << std::endl;
        return 1;
    }

    const std::string configPath = argv[1];

    LoadBalancerConfig config;
    if (YamlConfigSerializer serializer; !serializer.Load(config, configPath)) {
        std::cerr << "Error: Failed to load config from " << configPath << std::endl;
        return 1;
    }

    if (config.GetServiceCount() == 0) {
        std::cerr << "Error: No services defined in config" << std::endl;
        return 1;
    }

    std::cout << "Bit Bridge LB loaded config: " << config.GetName() << std::endl;
    std::cout << "  Listen: " << config.GetListenAddress() << ":" << config.GetListenPort() << std::endl;
    std::cout << "  Algorithm: " << config.GetRoutingAlgorithm() << std::endl;
    std::cout << "  Services: " << config.GetServiceCount() << std::endl;

    for (const auto &svc: config.GetServices()) {
        std::cout << "    - " << svc.GetName() << " (" << svc.GetIp() << ":" << svc.GetPort() << ")" << std::endl;
    }

    return 0;
}
