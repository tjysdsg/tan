#include "src/ast/ast_return.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

ASTReturn::ASTReturn(Token *token, size_t token_index) : ASTPrefix(token, token_index) {
  _type = ASTType::RET;
  _lbp = op_precedence[_type];
}

Value *ASTReturn::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  auto *result = _children[0]->codegen(compiler_session);
  if (_children[0]->is_lvalue()) {
    result = compiler_session->get_builder()->CreateLoad(result, "ret");
  }
  compiler_session->get_builder()->CreateRet(result);
  return nullptr;
}

std::string ASTReturn::get_type_name() const { return {}; }

std::shared_ptr<ASTTy> ASTReturn::get_ty() const { return nullptr; }

llvm::Type *ASTReturn::to_llvm_type(CompilerSession *) const { return nullptr; }

llvm::Metadata *ASTReturn::to_llvm_meta(CompilerSession *) const { return nullptr; }

} // namespace tanlang
