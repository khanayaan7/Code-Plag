#pragma once

#include "hashing.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace codeplag {

inline constexpr std::size_t kDefaultWindowSize = 4;

struct Fingerprint {
    std::uint64_t hash;
    int lineStart;
    int lineEnd;
};

std::vector<Fingerprint> winnow(
    const std::vector<KGramHash>& hashes,
    std::size_t windowSize = kDefaultWindowSize);

}  // namespace codeplag
