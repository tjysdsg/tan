#ifndef TAN_SRC_AST_SCOPE_H_
#define TAN_SRC_AST_SCOPE_H_
#include "base.h"

namespace tanlang {

AST_FWD_DECL(ASTNode);

struct Scope {
  umap<str, ASTNodePtr> _named{}; /// named identifiers in this scope
};

} // namespace tanlang

#endif //TAN_SRC_AST_SCOPE_H_
