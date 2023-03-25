#include "ast/decl.h"
#include "ast/context.h"

namespace tanlang {

template <typename Derived> void AnalysisAction<Derived>::push_scope(ASTBase *scope) { _scopes.push_back(scope); }

template <typename Derived> void AnalysisAction<Derived>::pop_scope() {
  TAN_ASSERT(!_scopes.empty());
  _scopes.pop_back();
}

template <typename Derived> Context *AnalysisAction<Derived>::ctx() {
  TAN_ASSERT(!_scopes.empty());
  return _scopes.back()->ctx();
}

template <typename Derived> Context *AnalysisAction<Derived>::top_ctx() {
  TAN_ASSERT(!_scopes.empty());
  return _scopes.front()->ctx();
}

template <typename Derived> Decl *AnalysisAction<Derived>::search_decl_in_scopes(const str &name) {
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

template <typename Derived> Loop *AnalysisAction<Derived>::search_loop_in_parent_scopes() {
  int n = (int)_scopes.size();
  TAN_ASSERT(n);
  for (int i = n - 1; i >= 0; --i) {
    auto *node = _scopes[(size_t)i];
    if (node->get_node_type() == ASTNodeType::LOOP) {
      return ast_cast<Loop>(node);
    }
  }

  return nullptr;
}

} // namespace tanlang
