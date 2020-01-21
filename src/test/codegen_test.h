#ifndef TAN_CODEGEN_TEST_H
#define TAN_CODEGEN_TEST_H

#include "ast.h"
#include "parser.h"
#include <gtest/gtest.h>
#include <iostream>

using tanlang::Reader;
using tanlang::tokenize;
using tanlang::Parser;

TEST(codegen, arithmatic) {
  std::string code = "1 + 2 * 3 / 4";
  Reader r;
  r.from_string(code);
  auto tokens = tokenize(&r);

  Parser p(tokens);
  p.parse();
  p._root->printTree();
  auto *value = p._root->codegen(&p);

  for (auto *&t : tokens) {
    delete t;
    t = nullptr;
  }
}

#endif /* TAN_CODEGEN_TEST_H */
