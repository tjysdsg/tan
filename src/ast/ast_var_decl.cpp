#include "src/ast/ast_var_decl.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_ty.h"
#include "src/analysis/analysis.h"
#include "src/common.h"
#include "token.h"
#include "compiler_session.h"

using namespace tanlang;

size_t ASTVarDecl::_nud() {
  /// var name
  auto name = _parser->parse<ASTType::ID>(_end_index, true);
  _name = get_name(ast_cast<ASTIdentifier>(name));
  _children.push_back(name);
  if (_parser->at(_end_index)->value == ":") {
    ++_end_index;
    /// type
    _ty = ast_cast<ASTTy>(_parser->parse<ASTType::TY>(_end_index, true));
    set_is_lvalue(_ty, true);
    _is_type_resolved = true;
  } else { _ty = nullptr; }
  _children.push_back(_ty);
  if (_type == ASTType::VAR_DECL) { _cs->add(get_name(ptr_from_this()), ptr_from_this()); }
  return _end_index;
}

void ASTVarDecl::set_ty(std::shared_ptr<ASTTy> ty) {
  _ty = ty;
  TAN_ASSERT(_children.size() >= 2);
  _children[1] = _ty;
  _is_type_resolved = true;
}

bool ASTVarDecl::is_type_resolved() { return _is_type_resolved; }

size_t ASTVarDecl::nud() {
  _end_index = _start_index + 1; /// skip "var"
  return _nud();
}

Value *ASTVarDecl::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  if (!_is_type_resolved) { error("Unknown type"); }
  cs->set_current_debug_location(_token->l, _token->c);
  Type *type = _children[1]->to_llvm_type(cs);
  _llvm_value = create_block_alloca(builder->GetInsertBlock(), type, 1, _name);
  if (_type == ASTType::VAR_DECL) { /// don't do this for arguments
    auto *default_value = _children[1]->get_llvm_value(cs);
    if (default_value) { builder->CreateStore(default_value, _llvm_value); }
  }
  /// debug info
  {
    auto *current_di_scope = cs->get_current_di_scope();
    auto *arg_meta = _ty->to_llvm_meta(cs);
    auto *di_arg = cs->_di_builder
        ->createAutoVariable(current_di_scope, _name, cs->get_di_file(), (unsigned) _token->l + 1, (DIType *) arg_meta);
    cs->_di_builder
        ->insertDeclare(_llvm_value,
            di_arg,
            cs->_di_builder->createExpression(),
            llvm::DebugLoc::get((unsigned) _token->l + 1, (unsigned) _token->c + 1, current_di_scope),
            builder->GetInsertBlock());
  }
  return _llvm_value;
}

ASTVarDecl::ASTVarDecl(Token *token, size_t token_index) : ASTNode(ASTType::VAR_DECL, 0, 0, token, token_index) {
  _is_typed = true;
  _is_valued = true;
  _is_lvalue = true;
}

