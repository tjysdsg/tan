#ifndef __TAN_AST_SCOPED_H__
#define __TAN_AST_SCOPED_H__

#include "base.h"
#include "ast/context.h"

namespace tanlang {

/**
 * \brief Must be inherited by subclasses ASTBase to function properly
 */
class Scoped {
public:
  Context *ctx() {
    if (!_ctx)
      _ctx = new Context((ASTBase *)this); // context <-> AST node mapping
    return _ctx;
  }

private:
  Context *_ctx = nullptr;
};

} // namespace tanlang

#endif //__TAN_AST_SCOPED_H__
