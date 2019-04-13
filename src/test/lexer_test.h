#ifndef __TAN_TEST_LEXER_TEST_H__
#define __TAN_TEST_LEXER_TEST_H__
#include "lexer.h"
#include <gtest/gtest.h>
#include <sstream>

TEST(Lexer, test1) {
  tanlang::Lexer lx1;
  tanlang::Lexer lx2;
  std::ostringstream oss1;
  std::ostringstream oss2;
  lx1.read_string("0x2432ABFE");
  lx1.lex();
  oss1 << lx1;
  EXPECT_TRUE((oss1.str() == "TOKEN_TYPE: 8; Value: 607300606"));
  lx2.read_string("0X2432ABFE");
  lx2.lex();
  oss2 << lx2;
  EXPECT_TRUE((oss2.str() == "TOKEN_TYPE: 8; Value: 607300606"));
}

TEST(Lexer, test2) {
  tanlang::Lexer lx1;
  tanlang::Lexer lx2;
  lx1.read_string("0b1001010101");
  std::ostringstream oss1;
  std::ostringstream oss2;
  lx1.lex();
  oss1 << lx1;
  EXPECT_TRUE(oss1.str() == "TOKEN_TYPE: 8; Value: 597");
  lx2.read_string("0B1001010101");
  lx2.lex();
  oss2 << lx2;
  EXPECT_TRUE(oss2.str() == "TOKEN_TYPE: 8; Value: 597");
}

TEST(Lexer, test3) {
  tanlang::Lexer lx1;
  lx1.read_string("91029321");
  lx1.lex();
  std::ostringstream oss1;
  oss1 << lx1;
  EXPECT_TRUE(oss1.str() == "TOKEN_TYPE: 8; Value: 91029321");
}
#endif //__TAN_TEST_LEXER_TEST_H__
