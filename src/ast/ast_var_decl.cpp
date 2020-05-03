#include "src/ast/ast_var_decl.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_ty.h"
#include "src/common.h"
#include "compiler.h"
#include "token.h"
#include "compiler_session.h"

namespace tanlang {

ASTVarDecl::ASTVarDecl(Token *token, size_t token_index) : ASTNode(ASTType::VAR_DECL, 0, 0, token, token_index) {}

size_t ASTVarDecl::_nud() {
  /// var name
  auto name = _parser->parse<ASTType::ID>(_end_index, true);
  _name = ast_cast<ASTIdentifier>(name)->get_name();
  _children.push_back(name);
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
  TAN_ASSERT(_children.size() >= 2);
  _children[1] = _ty;
  _is_type_resolved = true;
}

bool ASTVarDecl::is_type_resolved() const { return _is_type_resolved; }

size_t ASTVarDecl::nud() {
  _end_index = _start_index + 1; /// skip "var"
  return _nud();
}

Value *ASTVarDecl::codegen(CompilerSession *cs) {
  if (!_is_type_resolved) { report_code_error(_token, "Unknown type"); }
  cs->set_current_debug_location(_token->l, _token->c);
  TAN_ASSERT(_children[0]->is_named());
  std::string name = this->get_name();
  Type *type = _children[1]->to_llvm_type(cs);
  _llvm_value = create_block_alloca(cs->get_builder()->GetInsertBlock(), type, name);
  if (_type == ASTType::VAR_DECL) { /// don't do this for arguments
    auto *default_value = _children[1]->get_llvm_value(cs);
    if (default_value) { cs->get_builder()->CreateStore(default_value, _llvm_value); }
  }
  /// debug info
  {
    auto *current_di_scope = cs->get_current_di_scope();
    auto *arg_meta = get_ty()->to_llvm_meta(cs);
    llvm::DILocalVariable *di_arg = cs->get_di_builder()
        ->createAutoVariable(current_di_scope,
            get_name(),
            cs->get_di_file(),
            (unsigned) _token->l + 1,
            (DIType *) arg_meta);
    cs->get_di_builder()
        ->insertDeclare(_llvm_value,
            di_arg,
            cs->get_di_builder()->createExpression(),
            llvm::DebugLoc::get((unsigned) _token->l + 1, (unsigned) _token->c + 1, current_di_scope),
            cs->get_builder()->GetInsertBlock());
  }
  return _llvm_value;
}

bool ASTVarDecl::is_typed() const {
  if (!_is_type_resolved) { report_code_error(_token, "Unknown type"); }
  return true;
}

bool ASTVarDecl::is_named() const { return true; }

bool ASTVarDecl::is_lvalue() const { return true; }

} // namespace tanlang
