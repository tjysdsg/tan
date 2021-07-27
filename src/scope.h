#ifndef TAN_SRC_AST_SCOPE_H_
#define TAN_SRC_AST_SCOPE_H_
#include "base.h"
#include "src/ast/fwd.h"

namespace tanlang {

struct Scope {
  umap<str, Decl *> _declared{}; /// named identifiers in this scope
};

} // namespace tanlang

#endif //TAN_SRC_AST_SCOPE_H_
