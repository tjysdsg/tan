#ifndef __TAN_ANALYSIS_REGISTER_TOP_LEVEL_DECLARATIONS_H__
#define __TAN_ANALYSIS_REGISTER_TOP_LEVEL_DECLARATIONS_H__

#include "analysis/analysis_action.h"

namespace tanlang {

class RegisterTopLevelDeclarations : public AnalysisAction<RegisterTopLevelDeclarations> {
public:
  RegisterTopLevelDeclarations() = delete;
  explicit RegisterTopLevelDeclarations(SourceManager *sm);

public:
  DECLARE_AST_VISITOR_IMPL(FunctionDecl);
  DECLARE_AST_VISITOR_IMPL(StructDecl);

private:
  friend CompilerAction<RegisterTopLevelDeclarations>;
  void run_impl(Program *p);
};

} // namespace tanlang

#endif //__TAN_ANALYSIS_REGISTER_TOP_LEVEL_DECLARATIONS_H__
