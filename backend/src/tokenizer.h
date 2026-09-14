#pragma once

#include <string>
#include <vector>

namespace codeplag {

struct Token {
    std::string text;
    int line;

    bool operator==(const Token& other) const {
        return text == other.text && line == other.line;
    }
};

std::vector<Token> tokenize(const std::string& source);

}  // namespace codeplag
