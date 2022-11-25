#ifndef TAN_SRC_AST_SCOPE_H_
#define TAN_SRC_AST_SCOPE_H_
#include "base.h"
#include "ast/fwd.h"

namespace tanlang {

struct Scope {
  /**
   * The control flow in current scope, used by break and continue
   * */
  Loop *_current_loop = nullptr;
  umap<str, Decl *> _declared{}; /// named identifiers in this scope
};

} // namespace tanlang

#endif //TAN_SRC_AST_SCOPE_H_
