#pragma once

#include "base.h"
#include "lexer/lexer.h"
#include "source_file/token.h"
#include "source_file/source_file.h"
#include <gtest/gtest.h>
#include <iostream>

using tanlang::SourceFile;
using tanlang::tokenize;
using tanlang::TokenType;

TEST(source_file, invalid_file) {
  SourceFile f;

  bool catched = false;
  try {
    f.open("FKJDLSJFKJSDKLJFKDSJFKJKSDKFJsxxxS");
  } catch (const tanlang::CompileException &e) {
    EXPECT_EQ(e.type(), tanlang::ErrorType::FILE_NOT_FOUND);
    catched = true;
  }

  EXPECT_TRUE(catched);
}

TEST(source_file, empty) {
  str code = "";
  SourceFile f;
  f.from_string(code);

  SrcLoc end = f.end();
  EXPECT_EQ(end.l, 0);
  EXPECT_EQ(end.c, 1);

  SrcLoc end1 = f.forward(end);
  EXPECT_EQ(end1.l, 0);
  EXPECT_EQ(end1.c, 1);

  end1++;
  EXPECT_EQ(end1.l, 0);
  EXPECT_EQ(end1.c, 1);

  ++end1;
  EXPECT_EQ(end1.l, 0);
  EXPECT_EQ(end1.c, 1);
}

TEST(SrcLoc, increment) {
  str code = "line1\nlineno2\n";
  SourceFile f;
  f.from_string(code);

  SrcLoc loc = f.begin();
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ((++loc).c, i + 1);
  }
  EXPECT_EQ(loc.l, 0);
  EXPECT_EQ(loc.c, 4);

  loc++;
  EXPECT_EQ(loc.l, 1);
  EXPECT_EQ(loc.c, 0);

  for (int i = 0; i < 6; ++i) {
    EXPECT_EQ((loc++).c, i);
  }
  EXPECT_EQ(loc.l, 1);
  EXPECT_EQ(loc.c, 6);

  ++loc;
  EXPECT_EQ(loc.l, 2);
  EXPECT_EQ(loc.c, 0);
}

TEST(SrcLoc, comparison) {
  str code = "line1\nlineno2\n";
  SourceFile f;
  f.from_string(code);

  SrcLoc loc = f.begin();
  EXPECT_TRUE(loc == f.begin());
  loc.l = 1;
  loc.c = 0;
  EXPECT_TRUE(loc > f.begin());
  EXPECT_TRUE(f.begin() < loc);
  EXPECT_TRUE(f.begin() <= loc);
  EXPECT_TRUE(f.begin() != loc);
}

TEST(SourceManager, empty) {
  TokenizedSourceFile sm("file", {});

  Token *tok = sm.get_token(0);
  EXPECT_EQ(tok->get_type(), TokenType::COMMENTS);
  EXPECT_EQ(tok->get_line(), 0);
  EXPECT_EQ(tok->get_col(), 0);
  EXPECT_EQ(tok->get_source_line(), "");
  EXPECT_EQ(tok->get_value(), "");

  EXPECT_EQ(sm.get_last_token(), tok);

  EXPECT_EQ(sm.is_eof(0), false);
  EXPECT_EQ(sm.is_eof(1), true);
}
