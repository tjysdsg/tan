#include "src/ast/ast_identifier.h"
#include "src/ast/ast_var_decl.h"
#include "compiler_session.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

Value *ASTIdentifier::_codegen(CompilerSession *cs) {
  auto var = ast_cast<ASTVarDecl>(cs->get(_name));
  if (!var) { report_code_error(_token, "Cannot find variable '" + _name + "' in current scope"); }
  _llvm_value = var->get_llvm_value(cs);
  return _llvm_value;
}

ASTNodePtr ASTIdentifier::get_referred(bool strict) const {
  if (!_referred) {
    _referred = _cs->get(_name);
    if (!_referred && strict) { report_code_error(_token, "Cannot find variable '" + _name + "' in current scope"); }
  }
  return _referred;
}

size_t ASTIdentifier::nud() {
  _end_index = _start_index + 1;
  get_referred(false);
  _ty = _referred ? _referred->get_ty() : nullptr;
  return _end_index;
}

bool ASTIdentifier::is_lvalue() const { return get_referred()->is_lvalue(); }

bool ASTIdentifier::is_named() const { return true; }

bool ASTIdentifier::is_typed() const { return get_referred()->is_typed(); }

ASTIdentifier::ASTIdentifier(Token *token, size_t token_index) : ASTNode(ASTType::ID, 0, 0, token, token_index) {
  _name = token->value;
}

str ASTIdentifier::to_string(bool print_prefix) const {
  if (print_prefix) { return ASTNode::to_string(print_prefix) + " " + _name; }
  else { return _name; }
}

} // namespace tanlang
