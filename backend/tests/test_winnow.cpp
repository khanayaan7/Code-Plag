#include "hashing.h"
#include "tokenizer.h"
#include "winnow.h"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <vector>

namespace {

std::vector<codeplag::KGramHash> makeHashes(
    const std::vector<std::uint64_t>& values) {
    std::vector<codeplag::KGramHash> hashes;
    for (std::size_t index = 0; index < values.size(); ++index) {
        const int line = static_cast<int>(index + 1);
        hashes.push_back({values[index], index, index + 4, line, line});
    }
    return hashes;
}

}  // namespace

TEST_CASE("Winnowing selects expected minima from distinct hashes") {
    const auto fingerprints = codeplag::winnow(makeHashes({9, 3, 5, 2, 8, 1, 7}));

    REQUIRE(fingerprints.size() == 2);
    CHECK(fingerprints[0].hash == 2);
    CHECK(fingerprints[0].lineStart == 4);
    CHECK(fingerprints[1].hash == 1);
    CHECK(fingerprints[1].lineStart == 6);
}

TEST_CASE("Equal minima use the rightmost occurrence") {
    const auto fingerprints = codeplag::winnow(makeHashes({5, 2, 2, 7, 8}));

    REQUIRE(fingerprints.size() == 1);
    CHECK(fingerprints[0].hash == 2);
    CHECK(fingerprints[0].lineStart == 3);
}

TEST_CASE("Overlapping windows do not reselect the same fingerprint") {
    const auto fingerprints = codeplag::winnow(makeHashes({1, 6, 7, 8, 9, 10}));

    REQUIRE(fingerprints.size() == 3);
    CHECK(fingerprints[0].hash == 1);
    CHECK(fingerprints[1].hash == 6);
    CHECK(fingerprints[2].hash == 7);
}

TEST_CASE("The same hash at different positions remains two fingerprints") {
    const auto fingerprints = codeplag::winnow(makeHashes({1, 9, 9, 9, 1}));

    REQUIRE(fingerprints.size() == 2);
    CHECK(fingerprints[0].lineStart == 1);
    CHECK(fingerprints[1].lineStart == 5);
}

TEST_CASE("A stream shorter than k produces no fingerprints") {
    const auto tokens = codeplag::tokenize("return 1;");
    const auto hashes = codeplag::hashKGrams(tokens);

    REQUIRE(hashes.empty());
    REQUIRE(codeplag::winnow(hashes).empty());
}

TEST_CASE("A hash stream shorter than the window still selects one minimum") {
    const auto fingerprints = codeplag::winnow(makeHashes({7, 3, 5}));

    REQUIRE(fingerprints.size() == 1);
    CHECK(fingerprints.front().hash == 3);
}
