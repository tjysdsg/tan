#ifndef TAN_LEXDEF_H
#define TAN_LEXDEF_H

#include "utils.h"
#include <array>
#include <string>

namespace tanlang {
    enum class TokenType {
        END = -1,
        COMMENTS,
        KEYWORD,
        INT,
        FLOAT,
        ID,          // identifier
        CHAR,        // character
        STRING,      // string literal
        PUNCTUATION, // , ; . ( ) { } etc.
        RELOP,       // relational operator
        UOP,         // unary operator
        BOP,         // binary operator
    };
    constexpr std::array KEYWORDS{
            "for", "while", "do", "if", "else", "fn", "var", "int", "float",
            "continue", "break", "let", "struct", "enum", "union", "switch", "case", "str",
    };
    constexpr std::array PUNCTUATIONS{
            '~', '!', '#', '%', '^', '&', '*', '(', ')', '-', '=', '+', '[', ']',
            '{', '}', '\\', '|', ';', ':', '\'', '"', ',', '.', '<', '>', '/', '?',
    };
    // any symbol in OP can both be an operator itself or the start of an
    // operator
    constexpr std::array OP{
            '~', '!', '%', '^', '&', '*', '-', '=', '+', '|', '<', '>', '/',
    };
    // any element in OP_SINGLE MUST be an operator itself
    constexpr std::array OP_SINGLE{',', '.'};
    constexpr const_map<const char *, TokenType, 34> OPERATION_VALUE_TYPE_MAP(
            // RELOP
            std::pair("==", TokenType::RELOP), std::pair("!=", TokenType::RELOP), std::pair(">=", TokenType::RELOP),
            std::pair("<=", TokenType::RELOP), std::pair(">", TokenType::RELOP), std::pair("<", TokenType::RELOP),
            std::pair("&&", TokenType::RELOP), std::pair("||", TokenType::RELOP),
            // UOP
            std::pair("~", TokenType::UOP),
            // BOP
            std::pair("%=", TokenType::BOP), std::pair("%", TokenType::BOP), std::pair("^=", TokenType::BOP),
            std::pair("^", TokenType::BOP), std::pair("&=", TokenType::BOP), std::pair("&", TokenType::BOP),
            std::pair("+=", TokenType::BOP), std::pair("+", TokenType::BOP), std::pair("-=", TokenType::BOP),
            std::pair("-", TokenType::BOP), std::pair("*=", TokenType::BOP), std::pair("*", TokenType::BOP),
            std::pair("/=", TokenType::BOP), std::pair("/", TokenType::BOP), std::pair("|=", TokenType::BOP),
            std::pair("|", TokenType::BOP), std::pair("<<=", TokenType::BOP), std::pair("<<", TokenType::BOP),
            std::pair(">>=", TokenType::BOP), std::pair(">>", TokenType::BOP), std::pair("!=", TokenType::BOP),
            std::pair(",", TokenType::BOP), std::pair(".", TokenType::BOP));
} // namespace tanlang

#endif /*TAN_LEXDEF_H*/
