#ifndef TAN_PARSER_TEST_H
#define TAN_PARSER_TEST_H

#include "lexer/lexer.h"
#include "lexer/token.h"
#include "ast/source_manager.h"
#include "compiler/compiler.h"
#include "lexer/reader.h"
#include "parser/parser.h"
#include "ast/ast_base.h"
#include "ast/ast_node_type.h"
#include "ast/stmt.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace tanlang;

ASTBase *parse_string(str code) {
  Reader reader;
  reader.from_string(code);
  auto result = tokenize(&reader);
  auto tokens = tokenize(&reader);

  auto *parser = new Parser(new SourceManager("test module", tokens));
  return parser->parse();
}

TEST(parser, function_decl) {
  str code = "pub fn greet_cj(is_cj: bool, big_smoke: str) : i32 {"
             "  // you picked the wrong house, fool\n"
             "  return 666;"
             "}";
  auto node = parse_string(code);
  EXPECT_EQ(ASTNodeType::PROGRAM, node->get_node_type());
  auto program = ast_cast<Program>(node);
  EXPECT_EQ(1, program->get_children_size());
  auto statement = program->get_child_at<CompoundStmt>(0);
  EXPECT_EQ(1, statement->get_children_size());
  EXPECT_EQ(ASTNodeType::FUNC_DECL, statement->get_child_at(0)->get_node_type());
}

#endif /* TAN_PARSER_TEST_H */
