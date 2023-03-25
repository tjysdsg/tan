#ifndef __TAN_INCLUDE_ANALYSIS_ANALYSIS_ACTION_H__
#define __TAN_INCLUDE_ANALYSIS_ANALYSIS_ACTION_H__

#include "common/compiler_action.h"

namespace tanlang {

class Decl;

template <typename Derived> class AnalysisAction : public CompilerAction<Derived> {
public:
  [[nodiscard]] SourceManager *get_sm() const { return _sm; };

protected:
  [[noreturn]] void error(ASTBase *p, const str &message);
  void push_scope(ASTBase *scope);
  void pop_scope();
  Context *ctx();
  Context *top_ctx();
  Decl *search_decl_in_scopes(const str &name);
  Loop *search_loop_in_parent_scopes();

private:
  friend Derived;
  AnalysisAction() = delete;
  explicit AnalysisAction(SourceManager *sm) : _sm(sm) {}

private:
  vector<ASTBase *> _scopes{};
  SourceManager *_sm = nullptr;
};

} // namespace tanlang

#include "analysis/analysis_action.hpp"

#endif //__TAN_INCLUDE_ANALYSIS_ANALYSIS_ACTION_H__
