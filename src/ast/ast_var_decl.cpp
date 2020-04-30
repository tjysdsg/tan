#include "src/ast/ast_var_decl.h"
#include "src/ast/ast_identifier.h"
#include "compiler.h"
#include "token.h"
#include "compiler_session.h"
#include "src/common.h"

namespace tanlang {

ASTVarDecl::ASTVarDecl(Token *token, size_t token_index) : ASTNode(ASTType::VAR_DECL, 0, 0, token, token_index) {}

size_t ASTVarDecl::_nud() {
  _children.push_back(_parser->parse<ASTType::ID>(_end_index, true)); /// name
  if (_parser->at(_end_index)->value == ":") {
    ++_end_index;
    /// type
    _ty = ast_cast<ASTTy>(_parser->parse<ASTType::TY>(_end_index, true));
    _ty->set_is_lvalue(true);
    _is_type_resolved = true;
  } else { _ty = nullptr; }
  _children.push_back(_ty);
  _cs->add(this->get_name(), this->shared_from_this());
  return _end_index;
}

void ASTVarDecl::set_ty(std::shared_ptr<ASTTy> ty) {
  _ty = ty;
  assert(_children.size() >= 2);
  _children[1] = _ty;
  _is_type_resolved = true;
}

bool ASTVarDecl::is_type_resolved() const { return _is_type_resolved; }

size_t ASTVarDecl::nud() {
  _end_index = _start_index + 1; /// skip "var"
  return _nud();
}

Value *ASTVarDecl::codegen(CompilerSession *compiler_session) {
  if (!_is_type_resolved) { report_code_error(_token, "Unknown type"); }
  compiler_session->set_current_debug_location(_token->l, _token->c);
  assert(_children[0]->is_named());
  std::string name = this->get_name();
  Type *type = ast_cast<ASTTy>(_children[1])->to_llvm_type(compiler_session);
  Value *var = create_block_alloca(compiler_session->get_builder()->GetInsertBlock(), type, name);
  this->_llvm_value = var;
  return _llvm_value;
}

std::string ASTVarDecl::get_name() const {
  auto n = ast_cast<ASTIdentifier>(_children[0]);
  assert(n);
  return n->get_name();
}

std::string ASTVarDecl::get_type_name() const {
  if (!_is_type_resolved) { report_code_error(_token, "Unknown type"); }
  return _children[1]->get_type_name();
}

std::shared_ptr<ASTTy> ASTVarDecl::get_ty() const {
  if (!_is_type_resolved) { report_code_error(_token, "Unknown type"); }
  return _ty;
}

llvm::Type *ASTVarDecl::to_llvm_type(CompilerSession *compiler_session) const {
  if (!_is_type_resolved) { report_code_error(_token, "Unknown type"); }
  auto t = ast_cast<ASTTy>(_children[1]);
  return t->to_llvm_type(compiler_session);
}

llvm::Value *ASTVarDecl::get_llvm_value(CompilerSession *) const { return _llvm_value; }

bool ASTVarDecl::is_typed() const {
  if (!_is_type_resolved) { report_code_error(_token, "Unknown type"); }
  return true;
}

bool ASTVarDecl::is_named() const { return true; }

bool ASTVarDecl::is_lvalue() const { return true; }

} // namespace tanlang
