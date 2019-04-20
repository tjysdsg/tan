#ifndef __TAN_TEST_LEXER_TEST_H__
#define __TAN_TEST_LEXER_TEST_H__
#include "lexer.h"
#include <gtest/gtest.h>
#include <sstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <sstream>
#include <ios>
#include <bitset>

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
    oss1 << lx1;
    EXPECT_TRUE((oss1.str() == "TOKEN_TYPE: 8; Value: 607300606;"));
    lx2.read_string("0X2432ABFE");
    lx2.lex();
    oss2 << lx2;
    EXPECT_TRUE((oss2.str() == "TOKEN_TYPE: 8; Value: 607300606;"));
    // test overflow
    constexpr uint64_t uint64_max = std::numeric_limits<uint64_t>::max();
    std::ostringstream stream;
    stream << std::hex << uint64_max;
    std::string uint64_max_str = "0x" + stream.str();
    lx3.read_string(uint64_max_str);
    lx3.lex();
    oss3 << lx3;
    EXPECT_TRUE(oss3.str() ==
                "TOKEN_TYPE: 8; Value: " + std::to_string(uint64_max) + ";");
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
    oss1 << lx1;
    EXPECT_TRUE(oss1.str() == "TOKEN_TYPE: 8; Value: 597;");
    lx2.read_string("0B1001010101");
    lx2.lex();
    oss2 << lx2;
    EXPECT_TRUE(oss2.str() == "TOKEN_TYPE: 8; Value: 597;");
    // test overflow
    constexpr uint64_t uint64_max = std::numeric_limits<uint64_t>::max();
    std::bitset<64> b(uint64_max);
    std::string uint64_max_str = "0b" + b.to_string();
    /////////
    lx3.read_string(uint64_max_str);
    lx3.lex();
    oss3 << lx3;
    EXPECT_TRUE(oss3.str() ==
                "TOKEN_TYPE: 8; Value: " + std::to_string(uint64_max) + ";");
}

TEST(Lexer, dec_number) {
    tanlang::Lexer lx1;
    lx1.read_string("91029321");
    lx1.lex();
    std::ostringstream oss1;
    oss1 << lx1;
    EXPECT_TRUE(oss1.str() == "TOKEN_TYPE: 8; Value: 91029321;");
}

TEST(Lexer, identifier) {
    tanlang::Lexer lx1;
    lx1.read_string("_shit996");
    lx1.lex();
    std::ostringstream oss1;
    oss1 << lx1;
    EXPECT_TRUE(oss1.str() == "TOKEN_TYPE: 4; Value: _shit996;");
    //
    tanlang::Lexer lx2;
    lx2.read_string("_shit__");
    lx2.lex();
    std::ostringstream oss2;
    oss2 << lx2;
    EXPECT_TRUE(oss2.str() == "TOKEN_TYPE: 4; Value: _shit__;");
}

TEST(Lexer, string_literal) {
    tanlang::Lexer lx1;
    lx1.read_string("\"shit fuck god damn\"");
    lx1.lex();
    std::ostringstream oss1;
    oss1 << lx1;
    EXPECT_TRUE(oss1.str() == "TOKEN_TYPE: 32; Value: shit fuck god damn;");
}

TEST(Lexer, character) {
    tanlang::Lexer lx1;
    lx1.read_string("'s'");
    lx1.lex();
    std::ostringstream oss1;
    oss1 << lx1;
    EXPECT_TRUE(oss1.str() == "TOKEN_TYPE: 64; Value: s;");
}

#endif //__TAN_TEST_LEXER_TEST_H__
