#include "src/ast/ast_parenthesis.h"
#include "compiler_session.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

Value *ASTParenthesis::_codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  _llvm_value = _children[0]->codegen(cs);
  size_t n = _children.size();
  for (size_t i = 1; i < n; ++i) { _children[i]->codegen(cs); }
  return _llvm_value;
}

size_t ASTParenthesis::nud() {
  _end_index = _start_index + 1; /// skip (
  while (true) {
    auto *t = _parser->at(_end_index);
    if (!t) {
      error("Unexpected EOF");
    } else if (t->type == TokenType::PUNCTUATION && t->value == ")") { /// end at )
      ++_end_index;
      break;
    }
    auto n = _parser->next_expression(_end_index, PREC_LOWEST);
    if (n) {
      _children.push_back(n);
    } else {
      error("Unexpected " + t->to_string());
    }
  }
  _ty = _children[0]->get_ty();
  return _end_index;
}

bool ASTParenthesis::is_typed() const {
  TAN_ASSERT(_children.size() > 0);
  return _children[0]->is_typed();
}

bool ASTParenthesis::is_lvalue() const {
  TAN_ASSERT(_children.size() > 0);
  return _children[0]->is_lvalue();
}

ASTParenthesis::ASTParenthesis(Token *token, size_t token_index) : ASTNode(ASTType::PARENTHESIS,
    op_precedence[ASTType::PARENTHESIS],
    0,
    token,
    token_index) {}

} // namespace tanlang
