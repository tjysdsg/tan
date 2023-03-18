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
  Context *add_ctx(str name) {
    auto *ret = new Context((ASTBase *)this);
    _scopes[name] = ret;
    return ret;
  }

  Context *ctx(str name) const {
    auto q = _scopes.find(name);
    if (q != _scopes.end()) {
      return q->second;
    }
    return nullptr;
  }

  Context *add_ctx() {
    auto *ret = new Context((ASTBase *)this);
    _scopes["default"] = ret;
    return ret;
  }

  Context *ctx() const {
    auto q = _scopes.find("default");
    if (q != _scopes.end()) {
      return q->second;
    }
    return nullptr;
  }

private:
  umap<str, Context *> _scopes;
};

} // namespace tanlang

#endif //__TAN_AST_SCOPED_H__
