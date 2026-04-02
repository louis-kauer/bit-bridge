#include "AppSettings.hpp"

#include <toml++/toml.hpp>
#include <filesystem>
#include <fstream>

AppSettings::AppSettings()
    : m_configFilePath("bitbridge-config.yaml")
      , m_defaultName("my-bit-bridge-lb")
      , m_defaultListenAddress("0.0.0.0")
      , m_defaultListenPort(8080)
      , m_defaultRoutingAlgorithm("p2c")
      , m_defaultHealthCheckIntervalMs(5000)
      , m_defaultHealthCheckTimeoutMs(2000)
      , m_defaultHealthCheckUnhealthyThreshold(3)
      , m_defaultMaxPerService(1024)
      , m_defaultIdleTimeoutMs(30000)
      , m_defaultConnectTimeoutMs(5000) {
}

bool AppSettings::LoadFromFile(const std::string &filePath) {
    if (!std::filesystem::exists(filePath)) {
        return SaveToFile(filePath);
    }

    try {
        toml::table tbl = toml::parse_file(filePath);

        m_configFilePath = tbl["config_file_path"].value_or(m_configFilePath);

        if (auto defaults = tbl["defaults"].as_table()) {
            m_defaultName = (*defaults)["name"].value_or(m_defaultName);
            m_defaultListenAddress = (*defaults)["listen_address"].value_or(m_defaultListenAddress);
            m_defaultListenPort = static_cast<uint16_t>((*defaults)["listen_port"].value_or(
                static_cast<int64_t>(m_defaultListenPort)));
            m_defaultRoutingAlgorithm = (*defaults)["routing_algorithm"].value_or(m_defaultRoutingAlgorithm);
        }

        if (auto hc = tbl["defaults"]["health_check"].as_table()) {
            m_defaultHealthCheckEnabled = (*hc)["enabled"].value_or(m_defaultHealthCheckEnabled);
            m_defaultHealthCheckIntervalMs = static_cast<uint32_t>((*hc)["interval_ms"].value_or(
                static_cast<int64_t>(m_defaultHealthCheckIntervalMs)));
            m_defaultHealthCheckTimeoutMs = static_cast<uint32_t>((*hc)["timeout_ms"].value_or(
                static_cast<int64_t>(m_defaultHealthCheckTimeoutMs)));
            m_defaultHealthCheckUnhealthyThreshold = static_cast<uint32_t>((*hc)["unhealthy_threshold"].value_or(
                static_cast<int64_t>(m_defaultHealthCheckUnhealthyThreshold)));
        }

        if (auto conn = tbl["defaults"]["connection"].as_table()) {
            m_defaultMaxPerService = static_cast<uint32_t>((*conn)["max_per_service"].value_or(
                static_cast<int64_t>(m_defaultMaxPerService)));
            m_defaultIdleTimeoutMs = static_cast<uint32_t>((*conn)["idle_timeout_ms"].value_or(
                static_cast<int64_t>(m_defaultIdleTimeoutMs)));
            m_defaultConnectTimeoutMs = static_cast<uint32_t>((*conn)["connect_timeout_ms"].value_or(
                static_cast<int64_t>(m_defaultConnectTimeoutMs)));
        }

        return true;
    } catch (const toml::parse_error &) {
        return false;
    }
}

bool AppSettings::SaveToFile(const std::string &filePath) const {
    try {
        std::ofstream fout(filePath);
        if (!fout.is_open()) {
            return false;
        }

        fout << "# Bit Bridge - User Interface Settings\n";
        fout << "# These values configure the UI defaults, not the load balancer itself.\n\n";
        fout << "# Path to the load balancer configuration file (YAML)\n";
        fout << "config_file_path = \"" << m_configFilePath << "\"\n\n";
        fout << "# Default values pre-filled when creating a new configuration\n";
        fout << "[defaults]\n";
        fout << "name = \"" << m_defaultName << "\"\n";
        fout << "listen_address = \"" << m_defaultListenAddress << "\"\n";
        fout << "listen_port = " << m_defaultListenPort << "\n";
        fout << "routing_algorithm = \"" << m_defaultRoutingAlgorithm << "\"\n\n";
        fout << "[defaults.health_check]\n";
        fout << "enabled = " << (m_defaultHealthCheckEnabled ? "true" : "false") << "\n";
        fout << "interval_ms = " << m_defaultHealthCheckIntervalMs << "\n";
        fout << "timeout_ms = " << m_defaultHealthCheckTimeoutMs << "\n";
        fout << "unhealthy_threshold = " << m_defaultHealthCheckUnhealthyThreshold << "\n\n";
        fout << "[defaults.connection]\n";
        fout << "max_per_service = " << m_defaultMaxPerService << "\n";
        fout << "idle_timeout_ms = " << m_defaultIdleTimeoutMs << "\n";
        fout << "connect_timeout_ms = " << m_defaultConnectTimeoutMs << "\n";

        return true;
    } catch (const std::ios_base::failure &) {
        return false;
    }
}

std::string AppSettings::GetConfigFilePath() const { return m_configFilePath; }

std::string AppSettings::GetDefaultName() const { return m_defaultName; }
std::string AppSettings::GetDefaultListenAddress() const { return m_defaultListenAddress; }
uint16_t AppSettings::GetDefaultListenPort() const { return m_defaultListenPort; }
std::string AppSettings::GetDefaultRoutingAlgorithm() const { return m_defaultRoutingAlgorithm; }

bool AppSettings::GetDefaultHealthCheckEnabled() const { return m_defaultHealthCheckEnabled; }
uint32_t AppSettings::GetDefaultHealthCheckIntervalMs() const { return m_defaultHealthCheckIntervalMs; }
uint32_t AppSettings::GetDefaultHealthCheckTimeoutMs() const { return m_defaultHealthCheckTimeoutMs; }
uint32_t AppSettings::GetDefaultHealthCheckUnhealthyThreshold() const { return m_defaultHealthCheckUnhealthyThreshold; }

uint32_t AppSettings::GetDefaultMaxPerService() const { return m_defaultMaxPerService; }
uint32_t AppSettings::GetDefaultIdleTimeoutMs() const { return m_defaultIdleTimeoutMs; }
uint32_t AppSettings::GetDefaultConnectTimeoutMs() const { return m_defaultConnectTimeoutMs; }