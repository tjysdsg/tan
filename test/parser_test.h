#ifndef TAN_PARSER_TEST_H
#define TAN_PARSER_TEST_H

#include "lexer/lexer.h"
#include "source_file/token.h"
#include "ast/source_manager.h"
#include "compiler/compiler.h"
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
  EXPECT_EQ(ASTNodeType::FUNC_DECL, program->get_children()[0]->get_node_type());
}

#endif /* TAN_PARSER_TEST_H */
