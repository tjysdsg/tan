#ifndef TAN_LEXER_TEST_H
#define TAN_LEXER_TEST_H

#include "lexer.h"
#include "token.h"
#include "reader.h"
#include <gtest/gtest.h>
#include <iostream>

using tanlang::Reader;
using tanlang::tokenize;
using tanlang::TokenType;

TEST(tokenize, empty) {
  std::string code = "";
  Reader r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 0);
}

TEST(tokenize, line_comment) {
  std::string code = "// this is a comment";
  Reader r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0]->type, TokenType::COMMENTS);
  EXPECT_EQ(result[0]->value, " this is a comment");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, string_literal) {
  std::string code = "\"hello world, motherfucker dsfs \nshit \t\"";
  Reader r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0]->type, TokenType::STRING);
  EXPECT_EQ(result[0]->value, "hello world, motherfucker dsfs shit \t");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, string_literal_escape) {
  std::vector<std::string>
      input = {R"raw("\"hello world")raw", R"raw("\\")raw", R"raw("\n")raw", R"raw("he says, \"fuck you\"\n")raw"};
  std::vector<std::string> output = {"\"hello world", "\\", "\n", "he says, \"fuck you\"\n"};
  for (size_t i = 0; i < input.size(); ++i) {
    std::string code = input[i];
    Reader r;
    r.from_string(code);
    auto result = tokenize(&r);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->type, TokenType::STRING);
    EXPECT_EQ(result[0]->value, output[i]);
    for (auto *&t : result) {
      delete t;
      t = nullptr;
    }
  }
}

TEST(tokenize, char_literal) {
  std::vector<std::string> input = {"'\\\\'", "'\\n'", "'\\t'", "'\\''", "'\\\"'"};
  std::vector<std::string> output = {"\\", "\n", "\t", "'", "\""};
  for (size_t i = 0; i < input.size(); ++i) {
    std::string code = input[i];
    Reader r;
    r.from_string(code);
    auto result = tokenize(&r);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->type, TokenType::CHAR);
    EXPECT_EQ(result[0]->value, output[i]);
    for (auto *&t : result) {
      delete t;
      t = nullptr;
    }
  }
}

TEST(tokenize, block_comment) {
  std::string code = "/* this is a comment */";
  Reader r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0]->type, TokenType::COMMENTS);
  EXPECT_EQ(result[0]->value, " this is a comment ");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, number_literal) {
  std::string code = "fuck;0b10010111+0xaBFd,";
  Reader r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 6);
  EXPECT_EQ((int) result[2]->type, (int) TokenType::INT);
  EXPECT_EQ(result[2]->value, "0b10010111");
  EXPECT_EQ((int) result[4]->type, (int) TokenType::INT);
  EXPECT_EQ(result[4]->value, "0xaBFd");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

#endif /* TAN_LEXER_TEST_H */
