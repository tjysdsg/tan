#ifndef __TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
#include "base.h"
#include "src/analysis/ast_helper.h"

namespace tanlang {

AST_FWD_DECL(ParsableASTNode);
AST_FWD_DECL(ASTNode);
AST_FWD_DECL(ASTTy);

class AnalyzerImpl {
public:
  AnalyzerImpl(CompilerSession *cs);
  void analyze(ParsableASTNodePtr &p);

private:
  void analyze_member_access(ParsableASTNodePtr &p);
  void analyze_intrinsic(ParsableASTNodePtr &p);
  void resolve_ty(const ASTTyPtr &p) const;

private:
  CompilerSession *_cs = nullptr;
  ASTHelper _h;
};

}

#endif //__TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
