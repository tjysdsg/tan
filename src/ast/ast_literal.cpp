#include "src/ast/ast_literal.h"

namespace tanlang {

ASTLiteral::ASTLiteral(ASTType op, int lbp, int rbp, Token *t, size_t ti) : ASTNode(op, lbp, rbp, t, ti) {}

bool ASTLiteral::is_typed() const { return true; }

} // namespace tanlang
