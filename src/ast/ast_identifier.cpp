#include "src/ast/ast_identifier.h"
#include <include/parser.h>

namespace tanlang {

Value *ASTIdentifier::codegen(CompilerSession *compiler_session) {
  auto var = std::reinterpret_pointer_cast<ASTVarDecl>(compiler_session->get(_name));
  if (!var) { report_code_error(_token, "Cannot find variable '" + _name + "' in current scope"); }
  auto *v = var->get_llvm_value(compiler_session);
  return v;
}

std::string ASTIdentifier::get_name() const {
  return _name;
}

std::string ASTIdentifier::to_string(bool print_prefix) const {
  if (print_prefix) { return ASTNode::to_string(print_prefix) + " " + _name; }
  else { return _name; }
}

} // namespace tanlang
