#include "YamlConfigSerializer.hpp"
#include "LoadBalancerConfig.hpp"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>

bool YamlConfigSerializer::Save(const LoadBalancerConfig &config, const std::string &filePath) {
    try {
        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "name" << YAML::Value << config.GetName();
        out << YAML::Key << "listenAddress" << YAML::Value << config.GetListenAddress();
        out << YAML::Key << "listenPort" << YAML::Value << config.GetListenPort();
        out << YAML::Key << "routingAlgorithm" << YAML::Value << config.GetRoutingAlgorithm();

        // Health check section
        out << YAML::Key << "healthCheck" << YAML::Value << YAML::BeginMap;
        const HealthCheckConfig &hc = config.GetHealthCheck();
        out << YAML::Key << "enabled" << YAML::Value << hc.GetEnabled();
        out << YAML::Key << "intervalMs" << YAML::Value << hc.GetIntervalMs();
        out << YAML::Key << "timeoutMs" << YAML::Value << hc.GetTimeoutMs();
        out << YAML::Key << "unhealthyThreshold" << YAML::Value << hc.GetUnhealthyThreshold();
        out << YAML::EndMap;

        // Connection section
        out << YAML::Key << "connection" << YAML::Value << YAML::BeginMap;
        const ConnectionConfig &conn = config.GetConnection();
        out << YAML::Key << "maxPerService" << YAML::Value << conn.GetMaxPerService();
        out << YAML::Key << "idleTimeoutMs" << YAML::Value << conn.GetIdleTimeoutMs();
        out << YAML::Key << "connectTimeoutMs" << YAML::Value << conn.GetConnectTimeoutMs();
        out << YAML::EndMap;

        // Services section
        out << YAML::Key << "services" << YAML::Value << YAML::BeginSeq;
        for (const auto &service: config.GetServices()) {
            out << YAML::BeginMap;
            out << YAML::Key << "name" << YAML::Value << service.GetName();
            out << YAML::Key << "ip" << YAML::Value << service.GetIp();
            out << YAML::Key << "port" << YAML::Value << service.GetPort();
            out << YAML::Key << "weight" << YAML::Value << service.GetWeight();
            out << YAML::EndMap;
        }
        out << YAML::EndSeq;

        out << YAML::EndMap;

        if (const std::filesystem::path path(filePath); path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream fout(filePath);
        if (!fout.is_open()) {
            return false;
        }
        fout << out.c_str();
        fout.close();
        return true;
    } catch (const YAML::Exception &) {
        return false;
    }
}

bool YamlConfigSerializer::Load(LoadBalancerConfig &config, const std::string &filePath) {
    try {
        YAML::Node root = YAML::LoadFile(filePath);
        config.ClearServices();

        if (root["name"]) {
            config.SetName(root["name"].as<std::string>());
        }
        if (root["listenAddress"]) {
            config.SetListenAddress(root["listenAddress"].as<std::string>());
        }
        if (root["listenPort"]) {
            config.SetListenPort(root["listenPort"].as<uint16_t>());
        }
        if (root["routingAlgorithm"]) {
            config.SetRoutingAlgorithm(root["routingAlgorithm"].as<std::string>());
        }

        if (root["healthCheck"]) {
            YAML::Node hcNode = root["healthCheck"];
            HealthCheckConfig hc;
            if (hcNode["enabled"]) { hc.SetEnabled(hcNode["enabled"].as<bool>()); }
            if (hcNode["intervalMs"]) { hc.SetIntervalMs(hcNode["intervalMs"].as<uint32_t>()); }
            if (hcNode["timeoutMs"]) { hc.SetTimeoutMs(hcNode["timeoutMs"].as<uint32_t>()); }
            if (hcNode["unhealthyThreshold"]) { hc.SetUnhealthyThreshold(hcNode["unhealthyThreshold"].as<uint32_t>()); }
            config.SetHealthCheck(hc);
        }

        if (root["connection"]) {
            YAML::Node connNode = root["connection"];
            ConnectionConfig conn;
            if (connNode["maxPerService"]) { conn.SetMaxPerService(connNode["maxPerService"].as<uint32_t>()); }
            if (connNode["idleTimeoutMs"]) { conn.SetIdleTimeoutMs(connNode["idleTimeoutMs"].as<uint32_t>()); }
            if (connNode["connectTimeoutMs"]) { conn.SetConnectTimeoutMs(connNode["connectTimeoutMs"].as<uint32_t>()); }
            config.SetConnection(conn);
        }

        if (root["services"]) {
            for (const auto &serviceNode: root["services"]) {
                std::string name = serviceNode["name"] ? serviceNode["name"].as<std::string>() : "";
                std::string ip = serviceNode["ip"] ? serviceNode["ip"].as<std::string>() : "";
                uint16_t port = serviceNode["port"] ? serviceNode["port"].as<uint16_t>() : 0;
                uint16_t weight = serviceNode["weight"] ? serviceNode["weight"].as<uint16_t>() : 1;
                config.AddService(ServiceNode(name, ip, port, weight));
            }
        }

        return true;
    } catch (const YAML::Exception &) {
        return false;
    }
}