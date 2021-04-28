#ifndef TAN_LEXER_TEST_H
#define TAN_LEXER_TEST_H

#include "lexer.h"
#include "token.h"
#include "compiler_session.h"
#include "compiler.h"
#include "reader.h"
#include "parser.h"
#include "src/ast/ast_node.h"
#include <gtest/gtest.h>
#include <iostream>

using tanlang::Reader;
using tanlang::Parser;
using tanlang::Compiler;
using tanlang::CompilerSession;
using tanlang::tokenize;
using tanlang::TokenType;
using tanlang::ASTNodePtr;
using tanlang::ASTType;

ASTNodePtr parse_string(str code) {
  Reader reader;
  reader.from_string(code);
  auto result = tokenize(&reader);
  auto tokens = tokenize(&reader);

  Compiler compiler("test module"); // FIXME: decouple parsing and compiling
  CompilerSession *cs = new CompilerSession("test module", Compiler::GetDefaultTargetMachine());
  auto *parser = new Parser(tokens, "test", cs);
  return parser->parse();
}

TEST(parser, function_decl) {
  str code = "pub fn greet_cj(is_cj: bool, big_smoke: str) : i32 {"
             "  // you picked the wrong house, fool\n"
             "  return 666;"
             "}";
  auto node = parse_string(code);
  EXPECT_EQ(1, node->_children.size());
  auto statement = node->_children[0];
  EXPECT_EQ(1, statement->_children.size());
  EXPECT_EQ(ASTType::FUNC_DECL, statement->_children[0]->_type);
}

#endif /* TAN_LEXER_TEST_H */