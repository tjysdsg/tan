#ifndef __TAN_SRC_LEXER_LEXER_H__
#define __TAN_SRC_LEXER_LEXER_H__
#include <cctype>
#include <cstdint>

namespace tanlang {
    enum TOKEN_TYPE : uint64_t {
        UNKOWN = 0,
        KEYWORD = 1ull,
        // XXX = 1ull << 1,
        ID = 1ull << 2,
        INT = 1ull << 3,
        FLOAT = 1ull << 4,
        STR_LITERAL = 1ull << 5,
        CHAR = 1ull << 6,
        // symbols
        EQ = 1ull << 7,                // =
        PLUS = 1ull << 8,              // +
        PLUS_EQ = PLUS | EQ,           // +=
        MINUS,                         // -
        MINUS_EQ = MINUS | EQ,         // -=
        EXCLAIM,                       // !
        EXCLAIM_EQ = EXCLAIM | EQ,     // !=
        TILDE,                         // ~
        TILDE_EQ = TILDE | EQ,         // ~=
        CARET,                         // ^
        CARET_EQ = CARET | EQ,         // ^=
        STAR,                          // *
        STAR_EQ = STAR | EQ,           // *=
        SLASH,                         // /
        SLASH_EQ = SLASH | EQ,         // /=
        PERCENT,                       // %
        PERCENT_EQ = SLASH | EQ,       // %=
        AND,                           // &
        AND_EQ = AND | EQ,             // &=
        BAR,                           // |
        BAR_EQ = BAR | EQ,             // |=
        LT,                            // <
        LE = LT | EQ,                  // <=
        GT,                            // >
        GE = GT | EQ,                  // >=
        DOUBLE_AND = AND + 1,          // &&
        DOUBLE_BAR = BAR + 1,          // ||
        DOUBLE_EQ = EQ + 1,            // ==
        DOUBLE_LT = LT + 1,            // <<
        DOUBLE_LT_EQ = DOUBLE_LT | EQ, // <<=
        DOUBLE_GT = GT + 1,            // >>
        DOUBLE_GT_EQ = DOUBLE_GT | EQ, // >>=
    };

    struct token_info {
        TOKEN_TYPE type = UNKOWN;
        union {
            std::string str{};
            uint64_t val;
            double fval;
        };
        token_info(){};
        ~token_info(){};
    };
} // namespace tanlang
#endif // __TAN_SRC_LEXER_LEXER_H__
