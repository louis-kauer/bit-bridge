#include "LoadBalancerConfig.hpp"
#include "YamlConfigSerializer.hpp"
#include "ServicePool.hpp"
#include "P2CStrategy.hpp"
#include "ConsistentHashStrategy.hpp"
#include "IRoutingStrategy.hpp"
#include "TcpProxy.hpp"
#include "HealthChecker.hpp"

#include <boost/asio.hpp>
#include <memory>
#include <print>
#include <string>
#include <unordered_map>
#include <functional>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::println(stderr, "Usage: bit_bridge_lb <config-file>");
        return 1;
    }

    try {
        const std::string configPath = argv[1];

        LoadBalancerConfig config;
        YamlConfigSerializer serializer;
        if (auto result = serializer.Load(config, configPath); !result.has_value()) {
            std::println(stderr, "Error: {}", result.error());
            return 1;
        }

        if (config.GetServiceCount() == 0) {
            std::println(stderr, "Error: No services defined in config");
            return 1;
        }

        ServicePool pool(config.GetServices());

        const std::unordered_map<std::string, std::function<std::unique_ptr<IRoutingStrategy>()> > strategyFactory{
            // NOSONAR cpp:S6045
            {"p2c", []() -> std::unique_ptr<IRoutingStrategy> { return std::make_unique<P2CStrategy>(); }},
            {
                "consistent-hash",
                [&pool]() -> std::unique_ptr<IRoutingStrategy> {
                    return std::make_unique<ConsistentHashStrategy>(pool);
                }
            },
        };

        const auto it = strategyFactory.find(config.GetRoutingAlgorithm());
        if (it == strategyFactory.end()) {
            std::println(stderr, "Error: unknown routing algorithm '{}'", config.GetRoutingAlgorithm());
            return 1;
        }
        std::unique_ptr<IRoutingStrategy> strategy = it->second();

        boost::asio::io_context ioContext;

        const auto &conn = config.GetConnection();
        TcpProxy proxy(
            ioContext,
            config.GetListenAddress(),
            config.GetListenPort(),
            pool,
            *strategy,
            conn.GetConnectTimeoutMs(),
            conn.GetIdleTimeoutMs()
        );

        const auto &hc = config.GetHealthCheck();
        std::unique_ptr<HealthChecker> healthChecker;
        if (hc.GetEnabled()) {
            healthChecker = std::make_unique<HealthChecker>(ioContext, pool, hc);
        }

        boost::asio::signal_set signals(ioContext, SIGINT, SIGTERM);
        signals.async_wait([&proxy, &healthChecker](const boost::system::error_code &ec, int signum) {
            if (ec) {
                return;
            }
            std::println("\nSignal {}: shutting down", signum);
            if (healthChecker) {
                healthChecker->Stop();
            }
            proxy.Stop();
        });

        proxy.Start();
        if (healthChecker) {
            healthChecker->Start();
            std::println("Health checker enabled (interval: {}ms)", hc.GetIntervalMs());
        }

        std::println("Bit Bridge LB - {}", config.GetName());
        std::println("  Listen:    {}:{}", config.GetListenAddress(), config.GetListenPort());
        std::println("  Algorithm: {}", config.GetRoutingAlgorithm());
        std::println("  Services:  {}", config.GetServiceCount());
        for (const auto &svc : config.GetServices()) {
            std::println("    - {} ({}:{})", svc.GetName(), svc.GetIp(), svc.GetPort());
        }

        ioContext.run();

        return 0;
    } catch (const std::exception &e) {
        std::println(stderr, "Fatal: {}", e.what());
        return 1;
    }
}