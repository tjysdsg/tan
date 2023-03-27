#ifndef __TAN_ANALYSIS_ANALYSIS_ACTION_H__
#define __TAN_ANALYSIS_ANALYSIS_ACTION_H__

#include "common/compiler_action.h"
#include "ast/decl.h"
#include "ast/context.h"

namespace tanlang {

template <typename Derived, typename Input, typename Output>
class AnalysisAction : public CompilerAction<Derived, Input, Output> {
public:
  using AnalysisActionType = AnalysisAction<Derived, Input, Output>;

  AnalysisAction() = delete;

  [[nodiscard]] SourceManager *get_sm() const { return _sm; };

protected:
  [[noreturn]] void error(ASTBase *p, const str &message) {
    Error err(get_sm()->get_filename(), get_sm()->get_token(p->loc()), message);
    err.raise();
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

  Loop *search_loop_in_parent_scopes() {
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

private:
  friend Derived;
  explicit AnalysisAction(SourceManager *sm) : _sm(sm) {}

private:
  vector<ASTBase *> _scopes{};
  SourceManager *_sm = nullptr;
};

} // namespace tanlang

#endif //__TAN_ANALYSIS_ANALYSIS_ACTION_H__
