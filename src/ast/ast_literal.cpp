#include "src/ast/ast_literal.h"

namespace tanlang {

ASTLiteral::ASTLiteral(ASTType op, int lbp, int rbp, Token *token, size_t token_index) : ASTNode(op,
    lbp,
    rbp,
    token,
    token_index) {}

bool ASTLiteral::is_typed() const { return true; }

} // namespace tanlang
