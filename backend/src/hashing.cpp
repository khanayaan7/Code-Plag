#include "hashing.h"

#include <algorithm>
#include <string>
#include <vector>

namespace codeplag {
namespace {

std::uint64_t tokenValue(const std::string& text) {
    // The extra one makes every byte contribute a non-zero coefficient, and the
    // trailing sentinel prevents boundaries such as ["ab", "c"] and ["a", "bc"]
    // from sharing the same token encoding.
    std::uint64_t value = 0;
    for (const unsigned char character : text) {
        value = value * kHashBase + static_cast<std::uint64_t>(character) + 1;
    }
    return value * kHashBase + 1;
}

}  // namespace

std::vector<KGramHash> hashKGrams(
    const std::vector<Token>& tokens,
    std::size_t gramSize) {
    if (gramSize == 0 || tokens.size() < gramSize) {
        return {};
    }

    std::vector<std::uint64_t> values;
    values.reserve(tokens.size());
    for (const auto& token : tokens) {
        values.push_back(tokenValue(token.text));
    }

    std::uint64_t leadingPower = 1;
    for (std::size_t index = 1; index < gramSize; ++index) {
        leadingPower *= kHashBase;
    }

    std::uint64_t rollingHash = 0;
    for (std::size_t index = 0; index < gramSize; ++index) {
        rollingHash = rollingHash * kHashBase + values[index];
    }

    std::vector<KGramHash> hashes;
    hashes.reserve(tokens.size() - gramSize + 1);

    for (std::size_t start = 0; start + gramSize <= tokens.size(); ++start) {
        const auto begin = tokens.begin() + static_cast<std::ptrdiff_t>(start);
        const auto end = begin + static_cast<std::ptrdiff_t>(gramSize);
        const auto [minimumLine, maximumLine] = std::minmax_element(
            begin, end, [](const Token& left, const Token& right) {
                return left.line < right.line;
            });

        hashes.push_back({
            rollingHash,
            start,
            start + gramSize - 1,
            minimumLine->line,
            maximumLine->line,
        });

        if (start + gramSize < tokens.size()) {
            rollingHash =
                (rollingHash - values[start] * leadingPower) * kHashBase +
                values[start + gramSize];
        }
    }

    return hashes;
}

}  // namespace codeplag
