#include "Xoshiro256.hpp"

Xoshiro256StarStar::Xoshiro256StarStar(uint64_t seed) {
    m_state[0] = SplitMix64(seed);
    m_state[1] = SplitMix64(m_state[0]);
    m_state[2] = SplitMix64(m_state[1]);
    m_state[3] = SplitMix64(m_state[2]);
}

Xoshiro256StarStar::result_type Xoshiro256StarStar::operator()() {
    const uint64_t result = RotateLeft(m_state[1] * 5, 7) * 9;
    const uint64_t t = m_state[1] << 17;

    m_state[2] ^= m_state[0];
    m_state[3] ^= m_state[1];
    m_state[1] ^= m_state[2];
    m_state[0] ^= m_state[3];

    m_state[2] ^= t;
    m_state[3] = RotateLeft(m_state[3], 45);

    return result;
}

constexpr uint64_t Xoshiro256StarStar::RotateLeft(const uint64_t x, const unsigned int k) {
    return (x << k) | (x >> (64 - k));
}

uint64_t Xoshiro256StarStar::SplitMix64(uint64_t &state) {
    uint64_t z = (state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}