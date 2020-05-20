#include "src/ast/ast_ty.h"
#include "src/analysis/analysis.h"
#include "src/common.h"
#include "token.h"
#include "compiler_session.h"

using namespace tanlang;

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
