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

TEST(Lexer, string_literal) {
    tanlang::Lexer lx1;
    lx1.read_string("\"shit fuck god damn\"");
    lx1.lex();
    while (tanlang::token_info *t = lx1.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::STR_LITERAL);
        EXPECT_EQ(t->str, "shit fuck god damn");
    }
}

TEST(Lexer, character) {
    tanlang::Lexer lx1;
    lx1.read_string("'s'");
    lx1.lex();
    while (tanlang::token_info *t = lx1.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::CHAR);
        EXPECT_EQ(t->val, 's');
    }
}

TEST(Lexer, symbol) {
    tanlang::Lexer lx;
    lx.read_string("<<");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::DOUBLE_LT);
    }
    //
    lx.read_string("<<=");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::DOUBLE_LT_EQ);
    } //
    lx.read_string("!=");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::EXCLAIM_EQ);
    }
    //
    lx.read_string("!");
    lx.lex();
    while (tanlang::token_info *t = lx.next_token()) {
        EXPECT_EQ(uint64_t(t->type), tanlang::EXCLAIM);
    }
    // TODO: add more test cases
}

#endif //__TAN_TEST_LEXER_TEST_H__
