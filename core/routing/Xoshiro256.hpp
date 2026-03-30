#ifndef BIT_BRIDGE_XOSHIRO256_HPP
#define BIT_BRIDGE_XOSHIRO256_HPP

#include <cstdint>
#include <limits>

class Xoshiro256StarStar {
public:
    using result_type = uint64_t;

    explicit Xoshiro256StarStar(uint64_t seed);

    static constexpr result_type min() { return 0; } // NOLINT(readability-identifier-naming)
    static constexpr result_type max() { return std::numeric_limits<uint64_t>::max(); } // NOLINT(readability-identifier-naming)

    result_type operator()();

private:
    uint64_t m_state[4]{};

    static constexpr uint64_t RotateLeft(uint64_t x, unsigned int k);

    static uint64_t SplitMix64(uint64_t &state);
};

#endif //BIT_BRIDGE_XOSHIRO256_HPP