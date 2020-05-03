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
  if (_children[0]->is_lvalue()) { result = compiler_session->get_builder()->CreateLoad(result, "ret"); }
  compiler_session->get_builder()->CreateRet(result);
  return nullptr;
}

size_t ASTReturn::nud() {
  auto ret = ASTPrefix::nud();
  _ty = nullptr;
  return ret;
}

} // namespace tanlang
