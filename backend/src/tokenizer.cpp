#include "tokenizer.h"

#include <cctype>
#include <string_view>
#include <unordered_set>

namespace codeplag {
namespace {

const std::unordered_set<std::string> kKeywords = {
    "if",       "else",      "for",      "while",   "do",
    "switch",   "case",      "break",    "continue", "return",
    "int",      "long",      "float",    "double",  "char",
    "bool",     "void",      "class",    "struct",  "public",
    "private",  "static",    "const",    "new",     "delete",
    "try",      "catch",     "throw",    "namespace", "using",
    "include",  "template",  "typename",
};

const std::unordered_set<std::string_view> kThreeCharacterOperators = {
    "<<=", ">>=", "<=>", "...", "->*",
};

const std::unordered_set<std::string_view> kTwoCharacterOperators = {
    "==", "!=", "<=", ">=", "++", "--", "&&", "||", "<<",
    ">>", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
    "->", "::", ".*", "##",
};

bool isIdentifierStart(char character) {
    const auto value = static_cast<unsigned char>(character);
    return std::isalpha(value) != 0 || character == '_';
}

bool isIdentifierPart(char character) {
    const auto value = static_cast<unsigned char>(character);
    return std::isalnum(value) != 0 || character == '_';
}

bool isDigit(char character) {
    return std::isdigit(static_cast<unsigned char>(character)) != 0;
}

bool startsNumericLiteral(const std::string& source, std::size_t position) {
    return isDigit(source[position]) ||
           (source[position] == '.' && position + 1 < source.size() &&
            isDigit(source[position + 1]));
}

std::size_t consumeNumericLiteral(const std::string& source, std::size_t position) {
    std::size_t current = position;

    while (current < source.size()) {
        const char character = source[current];
        const bool ordinaryPart =
            std::isalnum(static_cast<unsigned char>(character)) != 0 ||
            character == '_' || character == '.' || character == '\'';
        const bool exponentSign =
            (character == '+' || character == '-') && current > position &&
            (source[current - 1] == 'e' || source[current - 1] == 'E' ||
             source[current - 1] == 'p' || source[current - 1] == 'P');

        if (!ordinaryPart && !exponentSign) {
            break;
        }
        ++current;
    }

    return current;
}

}  // namespace

std::vector<Token> tokenize(const std::string& source) {
    std::vector<Token> tokens;
    std::size_t position = 0;
    int line = 1;

    while (position < source.size()) {
        const char character = source[position];

        if (character == '\n') {
            ++line;
            ++position;
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(character)) != 0) {
            ++position;
            continue;
        }

        if (character == '/' && position + 1 < source.size() &&
            source[position + 1] == '/') {
            position += 2;
            while (position < source.size() && source[position] != '\n') {
                ++position;
            }
            continue;
        }

        if (character == '/' && position + 1 < source.size() &&
            source[position + 1] == '*') {
            position += 2;
            while (position < source.size()) {
                if (source[position] == '\n') {
                    ++line;
                    ++position;
                } else if (source[position] == '*' && position + 1 < source.size() &&
                           source[position + 1] == '/') {
                    position += 2;
                    break;
                } else {
                    ++position;
                }
            }
            continue;
        }

        if (isIdentifierStart(character)) {
            const std::size_t start = position;
            while (position < source.size() && isIdentifierPart(source[position])) {
                ++position;
            }
            const std::string word = source.substr(start, position - start);
            tokens.push_back({kKeywords.count(word) != 0 ? word : "ID", line});
            continue;
        }

        if (startsNumericLiteral(source, position)) {
            const int tokenLine = line;
            position = consumeNumericLiteral(source, position);
            tokens.push_back({"NUM", tokenLine});
            continue;
        }

        if (character == '"' || character == '\'') {
            const int tokenLine = line;
            const char delimiter = character;
            ++position;
            while (position < source.size()) {
                if (source[position] == '\\' && position + 1 < source.size()) {
                    if (source[position + 1] == '\n') {
                        ++line;
                    }
                    position += 2;
                    continue;
                }
                if (source[position] == delimiter) {
                    ++position;
                    break;
                }
                if (source[position] == '\n') {
                    ++line;
                }
                ++position;
            }
            tokens.push_back({"STR", tokenLine});
            continue;
        }

        if (position + 3 <= source.size()) {
            const std::string_view candidate(source.data() + position, 3);
            if (kThreeCharacterOperators.count(candidate) != 0) {
                tokens.push_back({std::string(candidate), line});
                position += 3;
                continue;
            }
        }
        if (position + 2 <= source.size()) {
            const std::string_view candidate(source.data() + position, 2);
            if (kTwoCharacterOperators.count(candidate) != 0) {
                tokens.push_back({std::string(candidate), line});
                position += 2;
                continue;
            }
        }

        tokens.push_back({std::string(1, character), line});
        ++position;
    }

    return tokens;
}

}  // namespace codeplag
