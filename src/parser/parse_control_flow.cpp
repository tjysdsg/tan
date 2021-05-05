#include "parser.h"
#include "base.h"
#include "compiler_session.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_control_flow.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_node.h"
#include "src/ast/ast_member_access.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "src/common.h"
#include "intrinsic.h"
#include "token.h"
#include <memory>
#include <utility>

using namespace tanlang;

size_t ParserImpl::parse_if(const ParsableASTNodePtr &p) {
  auto pif = ast_cast<ASTIf>(p);
  TAN_ASSERT(pif);
  ++pif->_end_index; /// skip "if"
  /// condition
  auto condition = peek(pif->_end_index, TokenType::PUNCTUATION, "(");
  pif->_end_index = parse_node(condition);
  pif->append_child(condition);
  /// if clause
  auto if_clause = peek(pif->_end_index, TokenType::PUNCTUATION, "{");
  pif->_end_index = parse_node(if_clause);
  pif->append_child(if_clause);

  /// else clause, if any
  auto *token = at(pif->_end_index);
  if (token->type == TokenType::KEYWORD && token->value == "else") {
    auto else_clause = peek(pif->_end_index);
    pif->_end_index = parse_node(else_clause);
    pif->append_child(else_clause);
    pif->_has_else = true;
  }
  return pif->_end_index;
}

size_t ParserImpl::parse_else(const ParsableASTNodePtr &p) {
  ++p->_end_index; /// skip "else"
  auto else_clause = peek(p->_end_index);
  p->_end_index = parse_node(else_clause);
  p->append_child(else_clause);
  return p->_end_index;
}

size_t ParserImpl::parse_loop(const ParsableASTNodePtr &p) {
  auto pl = ast_cast<ASTLoop>(p);
  TAN_ASSERT(pl);
  if (at(p->_end_index)->value == "for") {
    // TODO: implement for loop
    pl->_loop_type = ASTLoopType::FOR;
  } else if (at(p->_end_index)->value == "while") {
    pl->_loop_type = ASTLoopType::WHILE;
  } else {
    TAN_ASSERT(false);
  }
  ++p->_end_index; /// skip while/for
  switch (pl->_loop_type) {
    case ASTLoopType::WHILE:
      peek(p->_end_index, TokenType::PUNCTUATION, "(");
      p->append_child(next_expression(p->_end_index)); /// condition
      peek(p->_end_index, TokenType::PUNCTUATION, "{");
      p->append_child(next_expression(p->_end_index)); /// loop body
      break;
    case ASTLoopType::FOR:
      // TODO: implement for loop
      TAN_ASSERT(false);
      break;
  }
  return p->_end_index;
}
