#include "src/ast/ast_identifier.h"
#include "parser.h"

namespace tanlang {

Value *ASTIdentifier::codegen(CompilerSession *compiler_session) {
  auto var = ast_cast<ASTVarDecl>(compiler_session->get(_name));
  if (!var) { report_code_error(_token, "Cannot find variable '" + _name + "' in current scope"); }
  _llvm_value = var->get_llvm_value(compiler_session);
  return _llvm_value;
}

std::string ASTIdentifier::get_name() const {
  return _name;
}

std::string ASTIdentifier::to_string(bool print_prefix) const {
  if (print_prefix) { return ASTNode::to_string(print_prefix) + " " + _name; }
  else { return _name; }
}

llvm::Value *ASTIdentifier::get_llvm_value(CompilerSession *) const { return _llvm_value; }

ASTIdentifier::ASTIdentifier(Token *token, size_t token_index) : ASTNode(ASTType::ID, 0, 0, token, token_index) {
  _name = token->value;
}

bool ASTIdentifier::is_lvalue() const { return true; }

bool ASTIdentifier::is_named() const { return true; }

} // namespace tanlang
