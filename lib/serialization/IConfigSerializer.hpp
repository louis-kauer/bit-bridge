#ifndef BIT_BRIDGE_ICONFIG_SERIALIZER_HPP
#define BIT_BRIDGE_ICONFIG_SERIALIZER_HPP

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

    virtual bool Save(const LoadBalancerConfig &config, const std::string &filePath) = 0;

    virtual bool Load(LoadBalancerConfig &config, const std::string &filePath) = 0;
};

#endif //BIT_BRIDGE_ICONFIG_SERIALIZER_HPP