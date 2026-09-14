#include "hashing.h"

#include <catch2/catch_test_macros.hpp>

#include <vector>

TEST_CASE("Equal token k-grams have equal deterministic hashes") {
    const std::vector<codeplag::Token> first = {
        {"int", 1}, {"ID", 1}, {"=", 1}, {"NUM", 1}, {";", 1}};
    const std::vector<codeplag::Token> second = {
        {"int", 8}, {"ID", 8}, {"=", 9}, {"NUM", 9}, {";", 9}};

    REQUIRE(codeplag::hashKGrams(first).front().hash ==
            codeplag::hashKGrams(second).front().hash);
}

TEST_CASE("Different token k-grams produce different hashes in a spot check") {
    const std::vector<codeplag::Token> first = {
        {"int", 1}, {"ID", 1}, {"=", 1}, {"NUM", 1}, {";", 1}};
    const std::vector<codeplag::Token> second = {
        {"return", 1}, {"ID", 1}, {"+", 1}, {"NUM", 1}, {";", 1}};

    REQUIRE(codeplag::hashKGrams(first).front().hash !=
            codeplag::hashKGrams(second).front().hash);
}

TEST_CASE("K-gram token and line ranges are correct") {
    const std::vector<codeplag::Token> tokens = {
        {"int", 2}, {"ID", 2}, {"=", 2}, {"NUM", 3}, {";", 3},
        {"return", 5}, {"ID", 5}};

    const auto hashes = codeplag::hashKGrams(tokens);

    REQUIRE(hashes.size() == 3);
    CHECK(hashes[0].tokenStart == 0);
    CHECK(hashes[0].tokenEnd == 4);
    CHECK(hashes[0].lineStart == 2);
    CHECK(hashes[0].lineEnd == 3);
    CHECK(hashes[2].tokenStart == 2);
    CHECK(hashes[2].tokenEnd == 6);
    CHECK(hashes[2].lineStart == 2);
    CHECK(hashes[2].lineEnd == 5);
}

TEST_CASE("A stream shorter than the gram size has no hashes") {
    const std::vector<codeplag::Token> tokens = {
        {"return", 1}, {"NUM", 1}, {";", 1}};

    REQUIRE(codeplag::hashKGrams(tokens).empty());
}
