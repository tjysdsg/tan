#ifndef TAN_LEXER_TEST_H
#define TAN_LEXER_TEST_H

#include "lexer.h"
#include "token.h"
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
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
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
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
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
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
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
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, number_literal) {
  std::string code = "fuck;0b10010111+0xaBFd,";
  using tanlang::Reader;
  using tanlang::tokenize;
  using tanlang::TokenType;
  Reader r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 6);
  //    std::cout << (int) result[2]->type << '\n' << (int) TokenType::INT << '\n';
  EXPECT_EQ((int) result[2]->type, (int) TokenType::INT);
  EXPECT_EQ(result[2]->value, "0b10010111");
  EXPECT_EQ((int) result[4]->type, (int) TokenType::INT);
  EXPECT_EQ(result[4]->value, "0xaBFd");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, general_test1) {
  tanlang::Reader r;
  r.open("test_program.tan");
  auto result = tanlang::tokenize(&r);
  using TT=tanlang::TokenType;
  std::vector<std::pair<std::string, TT>> expected =
      {std::pair("#", TT::PUNCTUATION), std::pair("build_msg", TT::ID), std::pair(":", TT::PUNCTUATION),
          std::pair("str", TT::KEYWORD), std::pair(";", TT::PUNCTUATION), std::pair("#", TT::PUNCTUATION),
          std::pair("prog_version", TT::ID), std::pair(":", TT::PUNCTUATION), std::pair("u32", TT::KEYWORD),
          std::pair(";", TT::PUNCTUATION), std::pair("#", TT::PUNCTUATION), std::pair("before_build", TT::ID),
          std::pair("(", TT::PUNCTUATION), std::pair(")", TT::PUNCTUATION), std::pair("{", TT::PUNCTUATION),
          std::pair("println", TT::ID), std::pair("(", TT::PUNCTUATION),
          std::pair("Building version {u32} ...", TT::STRING), std::pair(",", TT::PUNCTUATION),
          std::pair("prog_version", TT::ID), std::pair(")", TT::PUNCTUATION), std::pair(";", TT::PUNCTUATION),
          std::pair("println", TT::ID), std::pair("(", TT::PUNCTUATION), std::pair("{str}", TT::STRING),
          std::pair(",", TT::PUNCTUATION), std::pair("build_msg", TT::ID), std::pair(")", TT::PUNCTUATION),
          std::pair(";", TT::PUNCTUATION), std::pair("}", TT::PUNCTUATION), std::pair("#", TT::PUNCTUATION),
          std::pair("after_build", TT::ID), std::pair("(", TT::PUNCTUATION), std::pair(")", TT::PUNCTUATION),
          std::pair("{", TT::PUNCTUATION), std::pair("println", TT::ID), std::pair("(", TT::PUNCTUATION),
          std::pair("Success!!", TT::STRING), std::pair(")", TT::PUNCTUATION), std::pair(";", TT::PUNCTUATION),
          std::pair("}", TT::PUNCTUATION), std::pair("fn", TT::KEYWORD), std::pair("main", TT::ID),
          std::pair("(", TT::PUNCTUATION), std::pair("argc", TT::ID), std::pair(":", TT::PUNCTUATION),
          std::pair("u32", TT::KEYWORD), std::pair(",", TT::PUNCTUATION), std::pair("argv", TT::ID),
          std::pair(":", TT::PUNCTUATION), std::pair("[", TT::PUNCTUATION), std::pair("[", TT::PUNCTUATION),
          std::pair("]", TT::PUNCTUATION), std::pair("]", TT::PUNCTUATION), std::pair(")", TT::PUNCTUATION),
          std::pair(":", TT::PUNCTUATION), std::pair("u32", TT::KEYWORD), std::pair("{", TT::PUNCTUATION),
          std::pair(" get all arguments", TT::COMMENTS), std::pair("args", TT::ID), std::pair(":", TT::PUNCTUATION),
          std::pair("[", TT::PUNCTUATION), std::pair("[", TT::PUNCTUATION), std::pair("]", TT::PUNCTUATION),
          std::pair("]", TT::PUNCTUATION), std::pair("=", TT::BOP), std::pair("Vec", TT::ID),
          std::pair(":", TT::PUNCTUATION), std::pair(":", TT::PUNCTUATION), std::pair("from_buffer", TT::ID),
          std::pair("(", TT::PUNCTUATION), std::pair("Buffer", TT::ID), std::pair(":", TT::PUNCTUATION),
          std::pair(":", TT::PUNCTUATION), std::pair("new", TT::ID), std::pair("(", TT::PUNCTUATION),
          std::pair("argc", TT::ID), std::pair("*", TT::BOP), std::pair("sizeof", TT::ID),
          std::pair("(", TT::PUNCTUATION), std::pair("*", TT::BOP), std::pair("str", TT::KEYWORD),
          std::pair(")", TT::PUNCTUATION), std::pair(")", TT::PUNCTUATION), std::pair(")", TT::PUNCTUATION),
          std::pair(";", TT::PUNCTUATION), std::pair(" this is redundant, but for illustration purposes", TT::COMMENTS),
          std::pair("for", TT::KEYWORD), std::pair("(", TT::PUNCTUATION), std::pair("i", TT::ID),
          std::pair(":", TT::PUNCTUATION), std::pair("u32", TT::KEYWORD), std::pair("=", TT::BOP),
          std::pair("0", TT::INT), std::pair(";", TT::PUNCTUATION), std::pair("i", TT::ID), std::pair("<", TT::RELOP),
          std::pair("argc", TT::ID), std::pair(";", TT::PUNCTUATION), std::pair("i", TT::ID), std::pair("+=", TT::BOP),
          std::pair("1", TT::INT), std::pair(")", TT::PUNCTUATION), std::pair("{", TT::PUNCTUATION),
          std::pair("args", TT::ID), std::pair("[", TT::PUNCTUATION), std::pair("i", TT::ID),
          std::pair("]", TT::PUNCTUATION), std::pair("=", TT::BOP), std::pair("argv", TT::ID),
          std::pair("[", TT::PUNCTUATION), std::pair("i", TT::ID), std::pair("]", TT::PUNCTUATION),
          std::pair(";", TT::PUNCTUATION), std::pair("}", TT::PUNCTUATION), std::pair(" parse arguments", TT::COMMENTS),
          std::pair("for", TT::KEYWORD), std::pair("(", TT::PUNCTUATION), std::pair("i", TT::ID),
          std::pair(":", TT::PUNCTUATION), std::pair("u32", TT::KEYWORD), std::pair("=", TT::BOP),
          std::pair("0", TT::INT), std::pair(";", TT::PUNCTUATION), std::pair("i", TT::ID), std::pair("<", TT::RELOP),
          std::pair("argc", TT::ID), std::pair(";", TT::PUNCTUATION), std::pair("i", TT::ID), std::pair("+=", TT::BOP),
          std::pair("1", TT::INT), std::pair(")", TT::PUNCTUATION), std::pair("{", TT::PUNCTUATION),
          std::pair("if", TT::KEYWORD), std::pair("(", TT::PUNCTUATION), std::pair("args", TT::ID),
          std::pair("[", TT::PUNCTUATION), std::pair("i", TT::ID), std::pair("]", TT::PUNCTUATION),
          std::pair("==", TT::RELOP), std::pair("-v", TT::STRING), std::pair("||", TT::RELOP),
          std::pair("args", TT::ID), std::pair("[", TT::PUNCTUATION), std::pair("i", TT::ID),
          std::pair("]", TT::PUNCTUATION), std::pair("==", TT::RELOP), std::pair("--version", TT::STRING),
          std::pair(")", TT::PUNCTUATION), std::pair("{", TT::PUNCTUATION), std::pair("println", TT::ID),
          std::pair("(", TT::PUNCTUATION), std::pair("Version: {}", TT::STRING), std::pair(",", TT::PUNCTUATION),
          std::pair("#", TT::PUNCTUATION), std::pair("prog_version", TT::ID), std::pair(")", TT::PUNCTUATION),
          std::pair(";", TT::PUNCTUATION), std::pair("return", TT::KEYWORD), std::pair("0", TT::INT),
          std::pair(";", TT::PUNCTUATION), std::pair("}", TT::PUNCTUATION), std::pair("if", TT::KEYWORD),
          std::pair("(", TT::PUNCTUATION), std::pair("args", TT::ID), std::pair("[", TT::PUNCTUATION),
          std::pair("i", TT::ID), std::pair("]", TT::PUNCTUATION), std::pair("==", TT::RELOP),
          std::pair("-h", TT::STRING), std::pair("||", TT::RELOP), std::pair("args", TT::ID),
          std::pair("[", TT::PUNCTUATION), std::pair("i", TT::ID), std::pair("]", TT::PUNCTUATION),
          std::pair("==", TT::RELOP), std::pair("--help", TT::STRING), std::pair(")", TT::PUNCTUATION),
          std::pair("{", TT::PUNCTUATION), std::pair("print", TT::ID), std::pair("(", TT::PUNCTUATION),
          std::pair("Avaiable options:\\n", TT::STRING), std::pair(")", TT::PUNCTUATION),
          std::pair(";", TT::PUNCTUATION), std::pair("println", TT::ID), std::pair("(", TT::PUNCTUATION),
          std::pair("use -v or --version see program version", TT::STRING), std::pair(")", TT::PUNCTUATION),
          std::pair(";", TT::PUNCTUATION), std::pair("return", TT::KEYWORD), std::pair("1", TT::INT),
          std::pair(";", TT::PUNCTUATION), std::pair("}", TT::PUNCTUATION), std::pair("}", TT::PUNCTUATION),
          std::pair("return", TT::KEYWORD), std::pair("0", TT::INT), std::pair(";", TT::PUNCTUATION),};
  for (size_t i = 0; i < expected.size(); ++i) {
    EXPECT_EQ(result[i]->type, expected[i].second);
    EXPECT_EQ(result[i]->value, expected[i].first);
  }
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

#endif /* TAN_LEXER_TEST_H */
