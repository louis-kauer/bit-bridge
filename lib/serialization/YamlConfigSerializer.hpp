#ifndef BIT_BRIDGE_YAML_CONFIG_SERIALIZER_HPP
#define BIT_BRIDGE_YAML_CONFIG_SERIALIZER_HPP

#include "IConfigSerializer.hpp"

class YamlConfigSerializer : public IConfigSerializer {
public:
    YamlConfigSerializer() = default;

    YamlConfigSerializer(const YamlConfigSerializer &other) = default;

    YamlConfigSerializer(YamlConfigSerializer &&other) noexcept = default;

    ~YamlConfigSerializer() override = default;

    YamlConfigSerializer &operator=(const YamlConfigSerializer &other) = default;

    YamlConfigSerializer &operator=(YamlConfigSerializer &&other) noexcept = default;

    [[nodiscard]] std::expected<void, std::string> Save(
        const LoadBalancerConfig &config, const std::string &filePath) override;

    [[nodiscard]] std::expected<void, std::string> Load(
        LoadBalancerConfig &config, const std::string &filePath) override;
};

#endif //BIT_BRIDGE_YAML_CONFIG_SERIALIZER_HPP