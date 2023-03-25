#ifndef __TAN_INCLUDE_ANALYSIS_ANALYSIS_ACTION_H__
#define __TAN_INCLUDE_ANALYSIS_ANALYSIS_ACTION_H__

#include "common/compiler_action.h"

namespace tanlang {

class Decl;

template <typename Derived> class AnalysisAction : public CompilerAction<Derived> {
protected:
  void push_scope(ASTBase *scope);
  void pop_scope();
  Context *ctx();
  Context *top_ctx();
  Decl *search_decl_in_scopes(const str &name);
  Loop *search_loop_in_parent_scopes();

private:
  vector<ASTBase *> _scopes{};
};

} // namespace tanlang

#include "analysis/analysis_action.hpp"

#endif //__TAN_INCLUDE_ANALYSIS_ANALYSIS_ACTION_H__
