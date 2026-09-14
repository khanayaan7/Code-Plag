#include "compare.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("Identical files have 100 percent similarity") {
    const std::string code = R"(
int sum(int first, int second) {
    int result = first + second;
    return result;
}
)";

    const auto result = codeplag::compareCode(code, code);

    REQUIRE(result.totalFingerprints1 > 0);
    REQUIRE(result.similarity == Catch::Approx(100.0));
    REQUIRE(result.matchedFingerprints == result.totalFingerprints1);
    REQUIRE_FALSE(result.matchedLines1.empty());
}

TEST_CASE("Identical repetitive files still have 100 percent similarity") {
    const std::string code = R"(
int repeat(int value) {
    value = value + 1;
    value = value + 1;
    value = value + 1;
    value = value + 1;
    value = value + 1;
    return value;
}
)";

    const auto result = codeplag::compareCode(code, code);

    REQUIRE(result.totalFingerprints1 > result.matchedFingerprints);
    REQUIRE(result.similarity == Catch::Approx(100.0));
}

TEST_CASE("Unrelated files have low similarity") {
    const std::string arithmetic = R"(
int calculate(int a, int b) {
    int product = a * b;
    return product + 42;
}
)";
    const std::string branching = R"(
void inspect(bool ready) {
    while (ready) {
        if (ready) break;
        else continue;
    }
}
)";

    const auto result = codeplag::compareCode(arithmetic, branching);

    REQUIRE(result.similarity <= 20.0);
}

TEST_CASE("Identifier renaming retains high similarity") {
    const std::string original = R"(
int sum(int first, int second) {
    int result = first + second;
    return result;
}
)";
    const std::string renamed = R"(
int combine(int x, int y) {
    int answer = x + y;
    return answer;
}
)";

    const auto result = codeplag::compareCode(original, renamed);

    REQUIRE(result.similarity == Catch::Approx(100.0));
}

TEST_CASE("Formatting comments and blank lines do not reduce similarity") {
    const std::string compact =
        "int square(int value){int result=value*value;return result;}";
    const std::string formatted = R"(
// The function computes a square.
int square ( int value )
{
    /* preserve the multiplication */
    int result = value * value;

    return result;
}
)";

    const auto result = codeplag::compareCode(compact, formatted);

    REQUIRE(result.similarity == Catch::Approx(100.0));
}

TEST_CASE("Empty input returns zero similarity without matches") {
    const auto oneEmpty = codeplag::compareCode("", "int main() { return 0; }");
    const auto bothEmpty = codeplag::compareCode("", "");

    CHECK(oneEmpty.similarity == 0.0);
    CHECK(oneEmpty.matchedLines1.empty());
    CHECK(oneEmpty.matchedLines2.empty());
    CHECK(bothEmpty.similarity == 0.0);
    CHECK(bothEmpty.matchedFingerprints == 0);
}

TEST_CASE("Input shorter than k tokens returns zero similarity") {
    const auto result = codeplag::compareCode("return 1;", "return 2;");

    CHECK(result.totalFingerprints1 == 0);
    CHECK(result.totalFingerprints2 == 0);
    CHECK(result.similarity == 0.0);
}

TEST_CASE("Matching line ranges are merged when overlapping or adjacent") {
    const std::vector<codeplag::Fingerprint> first = {
        {10, 3, 5}, {20, 6, 8}, {30, 12, 13}, {40, 20, 21}};
    const std::vector<codeplag::Fingerprint> second = {
        {10, 1, 2}, {20, 4, 6}, {30, 10, 11}};

    const auto result = codeplag::compareFingerprints(first, second);

    REQUIRE(result.matchedLines1 ==
            std::vector<codeplag::LineRange>{{3, 8}, {12, 13}});
    REQUIRE(result.matchedLines2 ==
            std::vector<codeplag::LineRange>{{1, 2}, {4, 6}, {10, 11}});
    REQUIRE(result.matchedFingerprints == 3);
    REQUIRE(result.similarity == Catch::Approx(100.0));
}
