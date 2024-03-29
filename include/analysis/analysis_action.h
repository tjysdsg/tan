#ifndef __TAN_ANALYSIS_ANALYSIS_ACTION_H__
#define __TAN_ANALYSIS_ANALYSIS_ACTION_H__

#include "ast/ast_node_type.h"
#include "common/compiler_action.h"
#include "ast/decl.h"
#include "ast/context.h"

namespace tanlang {

template <typename Derived, typename Input, typename Output>
class SemanticAnalysisAction : public CompilerAction<Derived, Input, Output> {
protected:
  [[noreturn]] void error(ErrorType type, ASTBase *p, const str &message) {
    auto *src = p->src();
    Error(type, src->get_token(p->start()), src->get_token(p->end()), message).raise();
  }

  void push_scope(ASTBase *scope) { _scopes.push_back(scope); }

  void pop_scope() {
    TAN_ASSERT(!_scopes.empty());
    _scopes.pop_back();
  }

  Context *ctx() {
    TAN_ASSERT(!_scopes.empty());
    return _scopes.back()->ctx();
  }

  Context *top_ctx() {
    TAN_ASSERT(!_scopes.empty());
    return _scopes.front()->ctx();
  }

  Decl *search_decl_in_scopes(const str &name) {
    int n = (int)_scopes.size();
    TAN_ASSERT(n);
    Decl *ret = nullptr;
    for (int i = n - 1; i >= 0; --i) {
      Context *c = _scopes[(size_t)i]->ctx();
      ret = c->get_decl(name);
      if (ret)
        return ret;
    }

    return ret;
  }

  template <typename T, ASTNodeType node_type> T *search_node_in_parent_scopes() {
    int n = (int)_scopes.size();
    TAN_ASSERT(n);
    for (int i = n - 1; i >= 0; --i) {
      auto *node = _scopes[(size_t)i];
      if (node->get_node_type() == node_type) {
        return pcast<T>(node);
      }
    }

    return nullptr;
  }

private:
  friend Derived;
  SemanticAnalysisAction() = default;

private:
  vector<ASTBase *> _scopes{};
};

} // namespace tanlang

#endif //__TAN_ANALYSIS_ANALYSIS_ACTION_H__
