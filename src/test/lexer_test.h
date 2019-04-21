#ifndef __TAN_TEST_LEXER_TEST_H__
#define __TAN_TEST_LEXER_TEST_H__
#include "lexer.h"
#include "src/lexer/lexer_internal.h"
#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <ios>
#include <bitset>

TEST(Lexer, hex_number) {
    tanlang::Lexer lx;
    lx.read_string("0x2432ABFE");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::INT);
        EXPECT_EQ(t->val, 607300606);
    }
    lx.read_string("0X2432ABFE");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::INT);
        EXPECT_EQ(t->val, 607300606);
    }
    // test overflow
    constexpr uint64_t uint64_max = std::numeric_limits<uint64_t>::max();
    std::ostringstream stream;
    stream << std::hex << uint64_max;
    std::string uint64_max_str = "0x" + stream.str();
    lx.read_string(uint64_max_str);
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::INT);
        EXPECT_EQ(t->val, uint64_max);
    }
}

TEST(Lexer, binary_number) {
    tanlang::Lexer lx;
    lx.read_string("0b1001010101");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::INT);
        EXPECT_EQ(t->val, 597);
    }
    lx.read_string("0B1001010101");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::INT);
        EXPECT_EQ(t->val, 597);
    }
    // test overflow
    constexpr uint64_t uint64_max = std::numeric_limits<uint64_t>::max();
    std::bitset<64> b(uint64_max);
    std::string uint64_max_str = "0b" + b.to_string();
    /////////
    lx.read_string(uint64_max_str);
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::INT);
        EXPECT_EQ(t->val, uint64_max);
    }
}

TEST(Lexer, dec_number) {
    tanlang::Lexer lx;
    lx.read_string("91029321");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::INT);
        EXPECT_EQ(t->val, 91029321);
    }
}

TEST(Lexer, float_number) {
    tanlang::Lexer lx;
    lx.read_string("91.029e1");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::FLOAT);
        EXPECT_EQ(t->fval, 910.29);
    }
}

TEST(Lexer, identifier) {
    tanlang::Lexer lx;
    std::string str = "_shit996";
    lx.read_string(str);
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::ID);
        EXPECT_EQ(t->str, str);
    }
    //
    str = "_shit__";
    lx.read_string(str);
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::ID);
        EXPECT_EQ(t->str, str);
    }
}

#define _LEXER_TEST_CASE_(_str, tval, _type, _val)                             \
    do {                                                                       \
        lx.read_string(_str);                                                  \
        lx.lex();                                                              \
        while (tanlang::token_info *t = lx.next_token()) {                     \
            EXPECT_EQ(uint64_t(t->type), _type);                               \
            EXPECT_EQ(t->tval, _val);                                          \
        }                                                                      \
    } while (0)

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
