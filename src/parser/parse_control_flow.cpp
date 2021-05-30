#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/stmt.h"

using namespace tanlang;

size_t ParserImpl::parse_if(const ASTBasePtr &p) {
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

size_t ParserImpl::parse_else(const ASTBasePtr &p) {
  ++p->_end_index; /// skip "else"
  auto else_clause = peek(p->_end_index);
  p->_end_index = parse_node(else_clause);
  p->append_child(else_clause);
  return p->_end_index;
}

size_t ParserImpl::parse_loop(const ASTBasePtr &_p) {
  auto p = ast_must_cast<Loop>(_p);

  if (at(p->_end_index)->value == "for") {
    // TODO: implement for loop
    p->_loop_type = ASTLoopType::FOR;
  } else if (at(p->_end_index)->value == "while") {
    p->_loop_type = ASTLoopType::WHILE;
  } else {
    TAN_ASSERT(false);
  }
  ++p->_end_index; /// skip while/for
  switch (p->_loop_type) {
    case ASTLoopType::WHILE:
      /// predicate
      peek(p->_end_index, TokenType::PUNCTUATION, "(");
      auto _pred = next_expression(p->_end_index, p->get_lbp());
      ExprPtr pred = nullptr;
      if (!(pred = ast_cast<Expr>(_pred))) {
        error(p->_end_index, "Expect an expression");
      }
      p->set_predicate(pred);
      peek(p->_end_index, TokenType::PUNCTUATION, "{");

      /// loop body
      auto _body = next_expression(p->_end_index, p->get_lbp());
      StmtPtr body = nullptr;
      if (!(body = ast_cast<Stmt>(_body))) {
        error(p->_end_index, "Expect an expression");
      }
      p->set_body(body);
      break;
    case ASTLoopType::FOR:
      // TODO: implement for loop
      TAN_ASSERT(false);
      break;
  }
  return p->_end_index;
}
