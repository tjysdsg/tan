#include "ast_ty.h"

namespace tanlang {

ASTTy::ASTTy(Token *token) : ASTNode(ASTType::TY, 0, 0, token) {}

Ty ASTTy::get_ty() { return _ty; }

} // namespace tanlang
