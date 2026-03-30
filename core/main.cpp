#include "LoadBalancerConfig.hpp"
#include "YamlConfigSerializer.hpp"

#include <print>
#include <string>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::println(stderr, "Usage: bit_bridge_lb <config-file>");
        return 1;
    }

    const std::string configPath = argv[1];

    LoadBalancerConfig config;
    YamlConfigSerializer serializer;
    if (auto result = serializer.Load(config, configPath); !result) {
        std::println(stderr, "Error: {}", result.error());
        return 1;
    }

    if (config.GetServiceCount() == 0) {
        std::println(stderr, "Error: No services defined in config");
        return 1;
    }

    std::println("Bit Bridge LB loaded config: {}", config.GetName());
    std::println("  Listen: {}:{}", config.GetListenAddress(), config.GetListenPort());
    std::println("  Algorithm: {}", config.GetRoutingAlgorithm());
    std::println("  Services: {}", config.GetServiceCount());

    for (const auto &svc : config.GetServices()) {
        std::println("    - {} ({}:{})", svc.GetName(), svc.GetIp(), svc.GetPort());
    }

    return 0;
}