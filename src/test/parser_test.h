#ifndef TAN_PARSER_TEST_H
#define TAN_PARSER_TEST_H

#include "lexer.h"
#include "token.h"
#include "compiler_session.h"
#include "src/ast/source_manager.h"
#include "compiler.h"
#include "reader.h"
#include "parser.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_node_type.h"
#include "src/ast/stmt.h"
#include <gtest/gtest.h>
#include <iostream>

using namespace tanlang;

ASTBase *parse_string(str code) {
  Reader reader;
  reader.from_string(code);
  auto result = tokenize(&reader);
  auto tokens = tokenize(&reader);

  Compiler compiler("test module"); // FIXME: decouple parsing and compiling
  CompilerSession *cs = new CompilerSession("test module", Compiler::GetDefaultTargetMachine());
  cs->set_source_manager(new SourceManager("test module", tokens));
  auto *parser = new Parser(tokens, "test", cs);
  return parser->parse();
}

TEST(parser, function_decl) {
  str code = "pub fn greet_cj(is_cj: bool, big_smoke: str) : i32 {"
             "  // you picked the wrong house, fool\n"
             "  return 666;"
             "}";
  auto node = parse_string(code);
  EXPECT_EQ(ASTNodeType::PROGRAM, node->get_node_type());
  auto program = ast_must_cast<Program>(node);
  EXPECT_EQ(1, program->get_children_size());
  auto statement = ast_must_cast<CompoundStmt>(program->get_child_at(0));
  EXPECT_EQ(1, statement->get_children_size());
  EXPECT_EQ(ASTNodeType::FUNC_DECL, statement->get_child_at(0)->get_node_type());
}

#endif /* TAN_PARSER_TEST_H */
