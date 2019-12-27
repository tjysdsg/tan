#ifndef TAN_LEXER_TEST_H
#define TAN_LEXER_TEST_H

#include "lexer.h"
#include "reader.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(tokenize, line_comment) {
    std::string code = "// this is a comment";
    using tanlang::Reader;
    using tanlang::tokenize;
    using tanlang::TokenType;
    Reader r;
    r.from_string(code);
    auto result = tokenize(&r);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->type, TokenType::COMMENTS);
    EXPECT_EQ(result[0]->value, " this is a comment");
}

TEST(tokenize, string_literal) {
    std::string code = "\"hello world, motherfucker dsfs \nshit \t\"";
    using tanlang::Reader;
    using tanlang::tokenize;
    using tanlang::TokenType;
    Reader r;
    r.from_string(code);
    auto result = tokenize(&r);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->type, TokenType::STRING);
    EXPECT_EQ(result[0]->value, "hello world, motherfucker dsfs shit \t");
}

TEST(tokenize, char_literal) {
    std::string code = "'s'";
    using tanlang::Reader;
    using tanlang::tokenize;
    using tanlang::TokenType;
    Reader r;
    r.from_string(code);
    auto result = tokenize(&r);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->type, TokenType::CHAR);
    EXPECT_EQ(result[0]->value, "s");
}

TEST(tokenize, block_comment) {
    std::string code = "/* this is a comment */";
    using tanlang::Reader;
    using tanlang::tokenize;
    using tanlang::TokenType;
    Reader r;
    r.from_string(code);
    auto result = tokenize(&r);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->type, TokenType::COMMENTS);
    EXPECT_EQ(result[0]->value, " this is a comment ");
}

TEST(tokenize, number_literal) {
    std::string code = "fuck;0b10010111+0xaBFd,";
    using tanlang::Reader;
    using tanlang::tokenize;
    using tanlang::TokenType;
    Reader r;
    r.from_string(code);
    auto result = tokenize(&r);
    EXPECT_EQ(result.size(), 5);
//    std::cout << (int) result[2]->type << '\n' << (int) TokenType::INT << '\n';
    EXPECT_EQ((int) result[2]->type, (int) TokenType::INT);
    EXPECT_EQ(result[2]->value, "0b10010111");
    EXPECT_EQ((int) result[4]->type, (int) TokenType::INT);
    EXPECT_EQ(result[4]->value, "0xaBFd");
}

TEST(tokenize, general_test1) {
    tanlang::Reader r;
    r.open("test_program.tan");
    auto result = tanlang::tokenize(&r);
    for (auto e : result) {
        std::cout << e->value << std::endl;
    }
}

#endif /* TAN_LEXER_TEST_H */
