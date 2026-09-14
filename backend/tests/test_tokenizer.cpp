#include "tokenizer.h"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {

std::vector<std::string> texts(const std::vector<codeplag::Token>& tokens) {
    std::vector<std::string> result;
    result.reserve(tokens.size());
    for (const auto& token : tokens) {
        result.push_back(token.text);
    }
    return result;
}

}  // namespace

TEST_CASE("Tokenizer strips line and block comments") {
    const auto tokens = codeplag::tokenize(
        "int value = 1; // ignored identifiers and symbols\n"
        "/* ignored\nacross lines */ return value;");

    REQUIRE(texts(tokens) == std::vector<std::string>{
        "int", "ID", "=", "NUM", ";", "return", "ID", ";"});
    REQUIRE(tokens[5].line == 3);
}

TEST_CASE("Whitespace and formatting do not change normalized token text") {
    const auto compact = codeplag::tokenize("int main(){return 7;}");
    const auto formatted = codeplag::tokenize(
        "int   main ( )\n"
        "{\n"
        "  return 7 ;\n"
        "}");

    REQUIRE(texts(compact) == texts(formatted));
}

TEST_CASE("Literals and identifiers normalize while keywords remain") {
    const auto tokens = codeplag::tokenize(
        "const double total = 12.5e-2; char c = 'x'; return \"hello // world\";");

    REQUIRE(texts(tokens) == std::vector<std::string>{
        "const", "double", "ID", "=", "NUM", ";", "char", "ID", "=",
        "STR", ";", "return", "STR", ";"});
}

TEST_CASE("C++ digit separators remain inside numeric literals") {
    const auto tokens = codeplag::tokenize("long amount = 1'000'000;");

    REQUIRE(texts(tokens) ==
            std::vector<std::string>{"long", "ID", "=", "NUM", ";"});
}

TEST_CASE("Tokenizer tracks source lines for every token") {
    const auto tokens = codeplag::tokenize(
        "int first;\n"
        "\n"
        "/* comment\n"
        "   line */\n"
        "return first;");

    REQUIRE(tokens == std::vector<codeplag::Token>{
        {"int", 1}, {"ID", 1}, {";", 1},
        {"return", 5}, {"ID", 5}, {";", 5}});
}

TEST_CASE("Multi-character operators are preserved as single tokens") {
    const auto tokens = codeplag::tokenize("if (left == right && left != 0) ++left;");

    REQUIRE(texts(tokens) == std::vector<std::string>{
        "if", "(", "ID", "==", "ID", "&&", "ID", "!=", "NUM", ")",
        "++", "ID", ";"});
}
