#ifndef __TAN_TEST_LEXER_TEST_H__
#define __TAN_TEST_LEXER_TEST_H__
#include "lexer.h"
#include "src/lexer/lexer_internal.h"
#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <ios>
#include <bitset>

#define _LEXER_TEST_CASE_(_str, tval, _type, _val)                             \
    do {                                                                       \
        lx.read_string(_str);                                                  \
        lx.lex();                                                              \
        while (tanlang::token_info *t = lx.next_token()) {                     \
            EXPECT_EQ(uint64_t(t->type), _type);                               \
            EXPECT_EQ(t->tval, _val);                                          \
        }                                                                      \
    } while (0)

TEST(Lexer, hex_number) {
    tanlang::Lexer lx;
    using namespace tanlang;
    _LEXER_TEST_CASE_("0x2432ABFE", val, INT, 607300606);
    _LEXER_TEST_CASE_("0X2432ABFE", val, INT, 607300606);
    //
    constexpr uint64_t uint64_max = std::numeric_limits<uint64_t>::max();
    std::ostringstream stream;
    stream << std::hex << uint64_max;
    std::string uint64_max_str = "0x" + stream.str();
    _LEXER_TEST_CASE_(uint64_max_str, val, INT, 607300606);
}

TEST(Lexer, binary_number) {
    tanlang::Lexer lx;
    using namespace tanlang;
    _LEXER_TEST_CASE_("0b1001010101", val, INT, 597);
    _LEXER_TEST_CASE_("0B1001010101", val, INT, 597);
    //
    constexpr uint64_t uint64_max = std::numeric_limits<uint64_t>::max();
    std::bitset<64> b(uint64_max);
    std::string uint64_max_str = "0b" + b.to_string();
    _LEXER_TEST_CASE_(uint64_max_str, val, INT, 597);
}

TEST(Lexer, dec_number) {
    tanlang::Lexer lx;
    using namespace tanlang;
    _LEXER_TEST_CASE_("91029321", val, INT, 91029321);
    lx.read_string("hahah 0 is in the middle of the file 0");
    lx.lex();
    tanlang::token_info *t = lx.next_token(); // hahah
    t = lx.next_token();                      // 0
    EXPECT_EQ(uint64_t(t->type), tanlang::INT);
    EXPECT_EQ(t->val, 0);
}

TEST(Lexer, float_number) {
    tanlang::Lexer lx;
    using namespace tanlang;
    _LEXER_TEST_CASE_("91.029e1", fval, FLOAT, 910.29);
}

TEST(Lexer, identifier) {
    tanlang::Lexer lx;
    using namespace tanlang;
    _LEXER_TEST_CASE_("_shit996", str, ID, "_shit996");
    _LEXER_TEST_CASE_("_shit__", str, ID, "_shit__");
}

TEST(Lexer, string_literal) {
    tanlang::Lexer lx;
    using namespace tanlang;
    _LEXER_TEST_CASE_("\"shit fuck god damn\"", str, STR_LITERAL,
                      "shit fuck god damn");
    _LEXER_TEST_CASE_("\"fhdsk1-2-0riesd\t_fdslklj'\"", str, STR_LITERAL,
                      "fhdsk1-2-0riesd\t_fdslklj'");
    _LEXER_TEST_CASE_("\"string!\"\"string!\"", str, STR_LITERAL, "string!");
}

TEST(Lexer, character) {
    tanlang::Lexer lx;
    using namespace tanlang;
    _LEXER_TEST_CASE_("'s'", val, CHAR, 's');
    _LEXER_TEST_CASE_("'\\'", val, CHAR, '\\');
    _LEXER_TEST_CASE_("'c''c'", val, CHAR, 'c');
}

#define _LEXER_SYMBOL_TEST_CASE_(str, _type)                                   \
    do {                                                                       \
        lx.read_string(str);                                                   \
        lx.lex();                                                              \
        while (tanlang::token_info *t = lx.next_token()) {                     \
            EXPECT_EQ(uint64_t(t->type), _type);                               \
        }                                                                      \
    } while (0)

TEST(Lexer, symbol) {
    using namespace tanlang;
    tanlang::Lexer lx;
    _LEXER_SYMBOL_TEST_CASE_("=", EQ);
    _LEXER_SYMBOL_TEST_CASE_("+", PLUS);
    _LEXER_SYMBOL_TEST_CASE_("+=", PLUS_EQ);
    _LEXER_SYMBOL_TEST_CASE_("-", MINUS);
    _LEXER_SYMBOL_TEST_CASE_("-=", MINUS_EQ);
    _LEXER_SYMBOL_TEST_CASE_("!", EXCLAIM);
    _LEXER_SYMBOL_TEST_CASE_("!=", EXCLAIM_EQ);
    _LEXER_SYMBOL_TEST_CASE_("~", TILDE);
    _LEXER_SYMBOL_TEST_CASE_("~=", TILDE_EQ);
    _LEXER_SYMBOL_TEST_CASE_("^", CARET);
    _LEXER_SYMBOL_TEST_CASE_("^=", CARET_EQ);
    _LEXER_SYMBOL_TEST_CASE_("*", STAR);
    _LEXER_SYMBOL_TEST_CASE_("*=", STAR_EQ);
    _LEXER_SYMBOL_TEST_CASE_("/", SLASH);
    _LEXER_SYMBOL_TEST_CASE_("/=", SLASH_EQ);
    _LEXER_SYMBOL_TEST_CASE_("%", PERCENT);
    _LEXER_SYMBOL_TEST_CASE_("%=", PERCENT_EQ);
    _LEXER_SYMBOL_TEST_CASE_("&", AND);
    _LEXER_SYMBOL_TEST_CASE_("&=", AND_EQ);
    _LEXER_SYMBOL_TEST_CASE_("|", BAR);
    _LEXER_SYMBOL_TEST_CASE_("|=", BAR_EQ);
    _LEXER_SYMBOL_TEST_CASE_("<", LT);
    _LEXER_SYMBOL_TEST_CASE_("<<", DOUBLE_LT);
    _LEXER_SYMBOL_TEST_CASE_("<<=", DOUBLE_LT_EQ);
    _LEXER_SYMBOL_TEST_CASE_("<=", LE);
    _LEXER_SYMBOL_TEST_CASE_(">", GT);
    _LEXER_SYMBOL_TEST_CASE_(">>", DOUBLE_GT);
    _LEXER_SYMBOL_TEST_CASE_(">>=", DOUBLE_GT_EQ);
    _LEXER_SYMBOL_TEST_CASE_(">=", GE);
    _LEXER_SYMBOL_TEST_CASE_("&&", DOUBLE_AND);
    _LEXER_SYMBOL_TEST_CASE_("||", DOUBLE_BAR);
    _LEXER_SYMBOL_TEST_CASE_("==", DOUBLE_EQ);
}

#endif //__TAN_TEST_LEXER_TEST_H__
