#ifndef BIT_BRIDGE_APP_SETTINGS_HPP
#define BIT_BRIDGE_APP_SETTINGS_HPP

#include <string>
#include <cstdint> // needed for linux build

class AppSettings {
public:
    AppSettings();

    AppSettings(const AppSettings &other) = default;

    AppSettings(AppSettings &&other) noexcept = default;

    ~AppSettings() = default;

    AppSettings &operator=(const AppSettings &other) = default;

    AppSettings &operator=(AppSettings &&other) noexcept = default;

    [[nodiscard]] bool LoadFromFile(const std::string &filePath);

    [[nodiscard]] bool SaveToFile(const std::string &filePath) const;

    [[nodiscard]] std::string GetConfigFilePath() const;

    [[nodiscard]] std::string GetDefaultName() const;

    [[nodiscard]] std::string GetDefaultListenAddress() const;

    [[nodiscard]] uint16_t GetDefaultListenPort() const;

    [[nodiscard]] std::string GetDefaultRoutingAlgorithm() const;

    [[nodiscard]] bool GetDefaultHealthCheckEnabled() const;

    [[nodiscard]] uint32_t GetDefaultHealthCheckIntervalMs() const;

    [[nodiscard]] uint32_t GetDefaultHealthCheckTimeoutMs() const;

    [[nodiscard]] uint32_t GetDefaultHealthCheckUnhealthyThreshold() const;

    [[nodiscard]] uint32_t GetDefaultMaxPerService() const;

    [[nodiscard]] uint32_t GetDefaultIdleTimeoutMs() const;

    [[nodiscard]] uint32_t GetDefaultConnectTimeoutMs() const;

private:
    std::string m_configFilePath;
    std::string m_defaultName;
    std::string m_defaultListenAddress;
    uint16_t m_defaultListenPort = 0;
    std::string m_defaultRoutingAlgorithm;
    bool m_defaultHealthCheckEnabled = true;
    uint32_t m_defaultHealthCheckIntervalMs = 0;
    uint32_t m_defaultHealthCheckTimeoutMs = 0;
    uint32_t m_defaultHealthCheckUnhealthyThreshold = 0;
    uint32_t m_defaultMaxPerService = 0;
    uint32_t m_defaultIdleTimeoutMs = 0;
    uint32_t m_defaultConnectTimeoutMs = 0;
};

#endif //BIT_BRIDGE_APP_SETTINGS_HPP
