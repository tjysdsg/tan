#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/stmt.h"

using namespace tanlang;

size_t ParserImpl::parse_if(ASTBase *_p) {
  auto p = ast_must_cast<If>(_p);

  /// if then
  parse_if_then_branch(p);

  /// else or elif clause, if any
  while (at(p->_end_index)->value == "else") {
    ++p->_end_index; /// skip "else"
    if (at(p->_end_index)->value == "if") { /// elif
      p->_end_index = parse_if_then_branch(p);
    } else { /// else
      auto else_clause = peek(p->_end_index);
      p->_end_index = parse_node(else_clause);
      p->add_else_branch(expect_stmt(else_clause));
    }
  }
  return p->_end_index;
}

size_t ParserImpl::parse_if_then_branch(If *p) {
  ++p->_end_index; /// skip "if"

  /// predicate
  auto _pred = peek(p->_end_index, TokenType::PUNCTUATION, "(");
  p->_end_index = parse_node(_pred);
  Expr *pred = expect_expression(_pred);

  /// then clause
  auto _then = peek(p->_end_index, TokenType::PUNCTUATION, "{");
  p->_end_index = parse_node(_then);
  Stmt *then_clause = expect_stmt(_then);

  p->add_if_then_branch(pred, then_clause);
  return p->_end_index;
}

size_t ParserImpl::parse_loop(ASTBase *_p) {
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
      auto _pred = next_expression(p->_end_index, PREC_LOWEST);
      Expr *pred = expect_expression(_pred);
      p->set_predicate(pred);
      peek(p->_end_index, TokenType::PUNCTUATION, "{");

      /// loop body
      auto _body = next_expression(p->_end_index, PREC_LOWEST);
      Stmt *body = expect_stmt(_body);
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
