#ifndef __TAN_ANALYSIS_ANALYSIS_ACTION_H__
#define __TAN_ANALYSIS_ANALYSIS_ACTION_H__

#include "ast/ast_node_type.h"
#include "common/compiler_action.h"
#include "common/compilation_unit.h"
#include "ast/decl.h"
#include "ast/context.h"

namespace tanlang {

template <typename Derived, typename Output>
class SingleUnitAnalysisAction : public CompilerAction<Derived, CompilationUnit *, Output> {
public:
  using AnalysisActionType = SingleUnitAnalysisAction<Derived, Output>;

  [[nodiscard]] SourceManager *get_sm() const { return _sm; };

protected:
  void init(CompilationUnit *cu) override { _sm = cu->source_manager(); }

  [[noreturn]] void error(ErrorType type, ASTBase *p, const str &message) {
    Error(type, get_sm()->get_token(p->start()), get_sm()->get_token(p->end()), message).raise();
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
  SingleUnitAnalysisAction() = default;

private:
  vector<ASTBase *> _scopes{};
  SourceManager *_sm = nullptr;
};

} // namespace tanlang

#endif //__TAN_ANALYSIS_ANALYSIS_ACTION_H__
