#include "compiler_session.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

Value *ASTIdentifier::_codegen(CompilerSession *cs) {
  auto var = ast_cast<ASTVarDecl>(cs->get(_name));
  if (!var) { error("Cannot find variable '" + _name + "' in current scope"); }
  _llvm_value = var->get_llvm_value(cs);
  return _llvm_value;
}

str ASTIdentifier::to_string(bool print_prefix) {
  if (print_prefix) { return ASTNode::to_string(print_prefix) + " " + _name; }
  else { return _name; }
}

} // namespace tanlang
