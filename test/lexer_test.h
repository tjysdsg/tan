#ifndef TAN_LEXER_TEST_H
#define TAN_LEXER_TEST_H

#include "base.h"
#include "lexer/lexer.h"
#include "source_file/token.h"
#include "source_file/source_file.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace tanlang;

TEST(tokenize, empty) {
  str code = "";
  SourceFile r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 0);
}

TEST(tokenize, line_comment) {
  str code = "// this is a comment";
  SourceFile r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0]->get_type(), TokenType::COMMENTS);
  EXPECT_EQ(result[0]->get_value(), " this is a comment");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, string_literal) {
  str code = "\"hello world, motherfucker dsfs \nshit \t\"";
  SourceFile r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0]->get_type(), TokenType::STRING);
  EXPECT_EQ(result[0]->get_value(), "hello world, motherfucker dsfs \nshit \t");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, string_literal_escape) {
  vector<str> input = {R"raw("\"hello world")raw",
                       R"raw("\\")raw",
                       R"raw("\n")raw",
                       R"raw("\a")raw",
                       R"raw("\b")raw",
                       R"raw("\f")raw",
                       R"raw("\?")raw",
                       R"raw("\r")raw",
                       R"raw("\v")raw",
                       R"raw("\t")raw",
                       R"raw("he says, \"fuck you\"\n")raw"};
  vector<str> output = {"\"hello world",          "\\", "\n", "\a", "\b", "\f", "\?", "\r", "\v", "\t",
                        "he says, \"fuck you\"\n"};
  for (size_t i = 0; i < input.size(); ++i) {
    str code = input[i];
    SourceFile r;
    r.from_string(code);
    auto result = tokenize(&r);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->get_type(), TokenType::STRING);
    EXPECT_EQ(result[0]->get_value(), output[i]);
    for (auto *&t : result) {
      delete t;
      t = nullptr;
    }
  }
}

TEST(tokenize, char_literal) {
  vector<str> input = {"'\\\\'", "'\\n'", "'\\t'", "'\\''", "'\\\"'"};
  vector<str> output = {"\\", "\n", "\t", "'", "\""};
  for (size_t i = 0; i < input.size(); ++i) {
    str code = input[i];
    SourceFile r;
    r.from_string(code);
    auto result = tokenize(&r);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->get_type(), TokenType::CHAR);
    EXPECT_EQ(result[0]->get_value(), output[i]);
    for (auto *&t : result) {
      delete t;
      t = nullptr;
    }
  }
}

TEST(tokenize, block_comment1) {
  str code = "/* this is a comment */";
  SourceFile r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 1);
  EXPECT_EQ(result[0]->get_type(), TokenType::COMMENTS);
  EXPECT_EQ(result[0]->get_value(), " this is a comment ");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, block_comment2) {
  str code = "/*\n\nblock\n \tis a comment \n*/xxx";
  SourceFile r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ(result.size(), 2);
  EXPECT_EQ(result[0]->get_type(), TokenType::COMMENTS);
  EXPECT_EQ(result[0]->get_value(), "\nblock\nis a comment \n");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, number_literal) {
  str code = "0b10010111 + 0xaBFd -10 4.2 071";
  SourceFile r;
  r.from_string(code);
  auto result = tokenize(&r);

  EXPECT_EQ((int)result[0]->get_type(), (int)TokenType::INT);
  EXPECT_EQ(result[0]->get_value(), "0b10010111");

  EXPECT_EQ((int)result[2]->get_type(), (int)TokenType::INT);
  EXPECT_EQ(result[2]->get_value(), "0xaBFd");

  EXPECT_EQ((int)result[4]->get_type(), (int)TokenType::INT);
  EXPECT_EQ(result[4]->get_value(), "10");

  EXPECT_EQ((int)result[5]->get_type(), (int)TokenType::FLOAT);
  EXPECT_EQ(result[5]->get_value(), "4.2");

  EXPECT_EQ((int)result[6]->get_type(), (int)TokenType::INT);
  EXPECT_EQ(result[6]->get_value(), "071");
}

TEST(tokenize, number_literal_negative) {
  vector<str> inputs{"1a", "1b.", "1.1u"};
  for (const str &code : inputs) {
    SourceFile f;
    f.from_string(code);

    bool catched = false;
    try {
      tokenize(&f);
    } catch (const CompileException &e) {
      EXPECT_EQ(e.type(), ErrorType::SYNTAX_ERROR);
      catched = true;
    }
    EXPECT_TRUE(catched);
  }
}

TEST(tokenize, number_literal1) {
  str code = "1u + 2. - 3.0";
  SourceFile r;
  r.from_string(code);
  auto result = tokenize(&r);
  EXPECT_EQ((int)result[0]->get_type(), (int)TokenType::INT);
  EXPECT_EQ(result[0]->get_value(), "1u");
  EXPECT_EQ(result[0]->is_unsigned(), true);
  EXPECT_EQ((int)result[2]->get_type(), (int)TokenType::FLOAT);
  EXPECT_EQ(result[2]->get_value(), "2.");
  EXPECT_EQ((int)result[4]->get_type(), (int)TokenType::FLOAT);
  EXPECT_EQ(result[4]->get_value(), "3.0");
  for (auto *&t : result) {
    delete t;
    t = nullptr;
  }
}

TEST(tokenize, operators) {
  str input = "";
  for (const str &s : OP_ALL) {
    input += " " + s;
  }

  SourceFile r;
  r.from_string(input);
  auto result = tokenize(&r);

  EXPECT_EQ(result.size(), OP_ALL.size());
  for (size_t i = 0; i < OP_ALL.size(); ++i) {
    EXPECT_EQ(result[i]->get_type(), OPERATION_VALUE_TYPE_MAP[OP_ALL[i]]);
    EXPECT_EQ(result[i]->get_value(), OP_ALL[i]);
  }
}

#endif /* TAN_LEXER_TEST_H */
