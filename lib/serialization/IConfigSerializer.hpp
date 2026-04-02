#ifndef BIT_BRIDGE_ICONFIG_SERIALIZER_HPP
#define BIT_BRIDGE_ICONFIG_SERIALIZER_HPP

#include <expected>
#include <string>

class LoadBalancerConfig;

class IConfigSerializer {
public:
    IConfigSerializer() = default;

    IConfigSerializer(const IConfigSerializer &) = default;

    IConfigSerializer(IConfigSerializer &&) noexcept = default;

    virtual ~IConfigSerializer() = default;

    IConfigSerializer &operator=(const IConfigSerializer &) = default;

    IConfigSerializer &operator=(IConfigSerializer &&) noexcept = default;

    [[nodiscard]] virtual std::expected<void, std::string> Save(
        const LoadBalancerConfig &config, const std::string &filePath) = 0;

    [[nodiscard]] virtual std::expected<void, std::string> Load(
        LoadBalancerConfig &config, const std::string &filePath) = 0;
};

#endif //BIT_BRIDGE_ICONFIG_SERIALIZER_HPP