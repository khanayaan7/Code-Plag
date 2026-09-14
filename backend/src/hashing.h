#pragma once

#include "tokenizer.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace codeplag {

inline constexpr std::size_t kDefaultGramSize = 5;
inline constexpr std::uint64_t kHashBase = 131;

struct KGramHash {
    std::uint64_t hash;
    std::size_t tokenStart;
    std::size_t tokenEnd;
    int lineStart;
    int lineEnd;
};

std::vector<KGramHash> hashKGrams(
    const std::vector<Token>& tokens,
    std::size_t gramSize = kDefaultGramSize);

}  // namespace codeplag
