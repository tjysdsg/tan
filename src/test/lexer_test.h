#ifndef __TAN_TEST_LEXER_TEST_H__
#define __TAN_TEST_LEXER_TEST_H__
#include "lexer.h"
#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <sstream>
#include <ios>
#include <bitset>

namespace tanlang {
    struct token_info {
        uint64_t type;
        union {
            std::string str;
            uint64_t val;
            double fval;
        };
    };
} // namespace tanlang

TEST(Lexer, hex_number) {
    tanlang::Lexer lx1;
    tanlang::Lexer lx2;
    tanlang::Lexer lx3;
    tanlang::Lexer lx4;
    std::ostringstream oss1;
    std::ostringstream oss2;
    std::ostringstream oss3;
    ////
    lx1.read_string("0x2432ABFE");
    lx1.lex();
    while (tanlang::token_info *t = lx1.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 8);
        EXPECT_EQ(t->val, 607300606);
    }
    lx2.read_string("0X2432ABFE");
    lx2.lex();
    while (tanlang::token_info *t = lx2.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 8);
        EXPECT_EQ(t->val, 607300606);
    }
    // test overflow
    constexpr uint64_t uint64_max = std::numeric_limits<uint64_t>::max();
    std::ostringstream stream;
    stream << std::hex << uint64_max;
    std::string uint64_max_str = "0x" + stream.str();
    lx3.read_string(uint64_max_str);
    lx3.lex();
    while (tanlang::token_info *t = lx3.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 8);
        EXPECT_EQ(t->val, uint64_max);
    }
}

TEST(Lexer, binary_number) {
    tanlang::Lexer lx1;
    tanlang::Lexer lx2;
    tanlang::Lexer lx3;
    tanlang::Lexer lx4;
    std::ostringstream oss1;
    std::ostringstream oss2;
    std::ostringstream oss3;
    lx1.read_string("0b1001010101");
    lx1.lex();
    while (tanlang::token_info *t = lx1.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 8);
        EXPECT_EQ(t->val, 597);
    }
    lx2.read_string("0B1001010101");
    lx2.lex();
    while (tanlang::token_info *t = lx2.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 8);
        EXPECT_EQ(t->val, 597);
    }
    // test overflow
    constexpr uint64_t uint64_max = std::numeric_limits<uint64_t>::max();
    std::bitset<64> b(uint64_max);
    std::string uint64_max_str = "0b" + b.to_string();
    /////////
    lx3.read_string(uint64_max_str);
    lx3.lex();
    while (tanlang::token_info *t = lx3.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 8);
        EXPECT_EQ(t->val, uint64_max);
    }
}

TEST(Lexer, dec_number) {
    tanlang::Lexer lx1;
    lx1.read_string("91029321");
    lx1.lex();
    while (tanlang::token_info *t = lx1.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 8);
        EXPECT_EQ(t->val, 91029321);
    }
}

TEST(Lexer, float_number) {
    tanlang::Lexer lx1;
    lx1.read_string("91.029e1");
    lx1.lex();
    while (tanlang::token_info *t = lx1.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 16);
        EXPECT_EQ(t->fval, 910.29);
    }
}

TEST(Lexer, identifier) {
    tanlang::Lexer lx1;
    std::string str = "_shit996";
    lx1.read_string(str);
    lx1.lex();
    while (tanlang::token_info *t = lx1.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 4);
        EXPECT_EQ(t->str, str);
    }
    //
    str = "_shit__";
    tanlang::Lexer lx2;
    lx2.read_string(str);
    lx2.lex();
    while (tanlang::token_info *t = lx2.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 4);
        EXPECT_EQ(t->str, str);
    }
}

TEST(Lexer, string_literal) {
    tanlang::Lexer lx1;
    lx1.read_string("\"shit fuck god damn\"");
    lx1.lex();
    while (tanlang::token_info *t = lx1.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 32);
        EXPECT_EQ(t->str, "shit fuck god damn");
    }
}

TEST(Lexer, character) {
    tanlang::Lexer lx1;
    lx1.read_string("'s'");
    lx1.lex();
    while (tanlang::token_info *t = lx1.next_token()) {
        EXPECT_EQ(uint64_t(t->type), 64);
        EXPECT_EQ(t->val, 's');
    }
}

#endif //__TAN_TEST_LEXER_TEST_H__
