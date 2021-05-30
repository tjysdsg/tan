#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/stmt.h"

using namespace tanlang;

size_t ParserImpl::parse_if(const ASTBasePtr &_p) {
  auto p = ast_must_cast<If>(_p);

  ++p->_end_index; /// skip "if"

  /// predicate
  auto _pred = peek(p->_end_index, TokenType::PUNCTUATION, "(");
  p->_end_index = parse_node(_pred);
  ExprPtr pred = expect_expression(_pred);
  p->set_predicate(pred);

  /// then clause
  auto then_clause = peek(p->_end_index, TokenType::PUNCTUATION, "{");
  p->_end_index = parse_node(then_clause);
  p->set_then(expect_stmt(then_clause));

  /// else clause, if any
  auto *token = at(p->_end_index);
  if (token->type == TokenType::KEYWORD && token->value == "else") {
    ++p->_end_index; /// skip "else"
    auto else_clause = peek(p->_end_index);
    p->_end_index = parse_node(else_clause);
    p->set_else(expect_stmt(else_clause));
  }
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
    case ASTLoopType::WHILE: {
      /// predicate
      peek(p->_end_index, TokenType::PUNCTUATION, "(");
      auto _pred = next_expression(p->_end_index, p->get_lbp());
      ExprPtr pred = expect_expression(_pred);
      p->set_predicate(pred);
      peek(p->_end_index, TokenType::PUNCTUATION, "{");

      /// loop body
      auto _body = next_expression(p->_end_index, p->get_lbp());
      StmtPtr body = expect_stmt(_body);
      p->set_body(body);
      break;
    }
    case ASTLoopType::FOR:
      // TODO: implement for loop
      TAN_ASSERT(false);
      break;
  }
  return p->_end_index;
}
