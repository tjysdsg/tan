#ifndef __TAN_SRC_LEXER_LEXER_H__
#define __TAN_SRC_LEXER_LEXER_H__
#include <cctype>
#include <cstdint>

namespace tanlang {
    enum TOKEN_TYPE : uint64_t {
        UNKOWN = 0,
        KEYWORD = 1ull,
        // XXX = 1ull << 1,
        ID = 1ull << 2u,
        INT = 1ull << 3u,
        FLOAT = 1ull << 4u,
        STR_LITERAL = 1ull << 5u,
        CHAR = 1ull << 6u,
        // symbols
        EQ = 1ull << 7u,               // =
        PLUS = 1ull << 8u,             // +
        MINUS = 1ull << 9u,            // -
        EXCLAIM = 1ull << 10u,         // !
        TILDE = 1ull << 11u,           // ~
        CARET = 1ull << 12u,           // ^
        STAR = 1ull << 13u,            // *
        SLASH = 1ull << 14u,           // /
        PERCENT = 1ull << 15u,         // %
        AND = 1ull << 16u,             // &
        BAR = 1ull << 17u,             // |
        LT = 1ull << 18u,              // <
        GT = 1ull << 19u,              // >
                                       //
        PLUS_EQ = PLUS | EQ,           // +=
        MINUS_EQ = MINUS | EQ,         // -=
        EXCLAIM_EQ = EXCLAIM | EQ,     // !=
        TILDE_EQ = TILDE | EQ,         // ~=
        CARET_EQ = CARET | EQ,         // ^=
        STAR_EQ = STAR | EQ,           // *=
        SLASH_EQ = SLASH | EQ,         // /=
        PERCENT_EQ = PERCENT | EQ,     // %=
        AND_EQ = AND | EQ,             // &=
        BAR_EQ = BAR | EQ,             // |=
        LE = LT | EQ,                  // <=
        GE = GT | EQ,                  // >=
        DOUBLE_AND = AND + 1,          // &&
        DOUBLE_BAR = BAR + 1,          // ||
        DOUBLE_EQ = EQ + 1,            // ==
        DOUBLE_LT = LT + 1,            // <<
        DOUBLE_LT_EQ = DOUBLE_LT | EQ, // <<=
        DOUBLE_GT = GT + 1,            // >>
        DOUBLE_GT_EQ = DOUBLE_GT | EQ, // >>=
    };

#define SWITCH_CASE_TOKEN_TYPE(type)                                           \
case ((type)):                                                                 \
    ret = #type;                                                               \
    break

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
#ifdef DEBUG_ENABLED
    std::string get_token_type_name(TOKEN_TYPE type);
#endif
} // namespace tanlang
#endif // __TAN_SRC_LEXER_LEXER_H__
