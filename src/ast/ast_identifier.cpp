#include "src/ast/ast_identifier.h"
#include "src/ast/ast_var_decl.h"
#include "compiler.h"
#include "parser.h"
#include "compiler_session.h"

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

size_t ASTIdentifier::nud() {
  _end_index = _start_index + 1;
  return _end_index;
}

bool ASTIdentifier::is_lvalue() const { return get_referred()->is_lvalue(); }

bool ASTIdentifier::is_named() const { return true; }

bool ASTIdentifier::is_typed() const { return get_referred()->is_typed(); }

std::string ASTIdentifier::get_type_name() const { return get_referred()->get_type_name(); }

llvm::Type *ASTIdentifier::to_llvm_type(CompilerSession *cm) const { return get_referred()->to_llvm_type(cm); }

std::shared_ptr<ASTTy> ASTIdentifier::get_ty() const { return get_referred()->get_ty(); }

ASTNodePtr ASTIdentifier::get_referred() const {
  if (!_referred) {
    _referred = _cs->get(_name);
    assert(_referred);
  }
  return _referred;
}

} // namespace tanlang
