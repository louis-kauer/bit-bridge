#ifndef BIT_BRIDGE_FNV_HASH_HPP
#define BIT_BRIDGE_FNV_HASH_HPP

#include <cstdint>
#include <string_view>

namespace fnv {
    constexpr uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
    constexpr uint64_t FNV_PRIME = 1099511628211ULL;

    [[nodiscard]] inline uint64_t Hash64(const std::string_view data) {
        uint64_t hash = FNV_OFFSET_BASIS;
        for (const char c: data) {
            hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
            hash *= FNV_PRIME;
        }
        return hash;
    }
} // namespace fnv

#endif //BIT_BRIDGE_FNV_HASH_HPP