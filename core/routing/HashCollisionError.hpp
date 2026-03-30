#ifndef BIT_BRIDGE_HASH_COLLISION_ERROR_HPP
#define BIT_BRIDGE_HASH_COLLISION_ERROR_HPP

#include <stdexcept>
#include <string>

class HashCollisionError : public std::runtime_error {
public:
    explicit HashCollisionError(size_t vnodeIndex)
        : std::runtime_error("Hash ring collision detected at vnode " + std::to_string(vnodeIndex)) {
    }
};

#endif //BIT_BRIDGE_HASH_COLLISION_ERROR_HPP