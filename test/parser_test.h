#ifndef TAN_PARSER_TEST_H
#define TAN_PARSER_TEST_H

#include "lexer/lexer.h"
#include "source_file/token.h"
#include "source_file/tokenized_source_file.h"
#include "source_file/source_file.h"
#include "parser/parser.h"
#include "ast/ast_base.h"
#include "ast/ast_node_type.h"
#include "ast/stmt.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace tanlang;

ASTBase *parse_string(str code) {
  SourceFile src;
  src.from_string(code);
  auto result = tokenize(&src);
  auto tokens = tokenize(&src);

  auto *parser = new Parser(new TokenizedSourceFile("test module", tokens));
  return parser->parse();
}

void negative_test(str code, ErrorType err_type) {
  bool caught = false;
  try {
    parse_string(code);
  } catch (const CompileException &e) {
    caught = true;
    EXPECT_EQ(e.type(), err_type);
  }

  EXPECT_TRUE(caught);
}

TEST(parser, function_decl) {
  str code = "pub fn greet_cj(is_cj: bool, big_smoke: str) : i32 {"
             "  // you picked the wrong house, fool\n"
             "  return 666;"
             "}";
  auto node = parse_string(code);
  EXPECT_EQ(ASTNodeType::PROGRAM, node->get_node_type());
  auto program = pcast<Program>(node);
  EXPECT_EQ(1, program->get_children_size());
  EXPECT_EQ(ASTNodeType::FUNC_DECL, program->get_children()[0]->get_node_type());
}

TEST(parser, ast_repr) {
  // TODO: test parser ast repr
}

TEST(parser, simple) {
  negative_test("return 0:", ErrorType::SYNTAX_ERROR);
  negative_test("return 0]", ErrorType::SYNTAX_ERROR);
  negative_test("return 0)", ErrorType::SYNTAX_ERROR);
  negative_test("return 0}", ErrorType::SYNTAX_ERROR);
}

TEST(parser, if_stmt) {
  negative_test("if true", ErrorType::SYNTAX_ERROR);
  negative_test("if (true", ErrorType::SYNTAX_ERROR);
  negative_test("if (true) {", ErrorType::SYNTAX_ERROR);
  negative_test("if (true) {} e", ErrorType::SYNTAX_ERROR);
  negative_test("if (true) {} else ", ErrorType::SYNTAX_ERROR);
  negative_test("if (true) {} elif { ", ErrorType::SYNTAX_ERROR);
  negative_test("if (true) {} else }", ErrorType::SYNTAX_ERROR);
  negative_test("if (true);", ErrorType::SYNTAX_ERROR);
}

#endif /* TAN_PARSER_TEST_H */
