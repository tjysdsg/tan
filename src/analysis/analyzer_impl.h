#ifndef __TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
#include "base.h"
#include "src/llvm_include.h"

namespace tanlang {

class ParsableASTNode;
using ParsableASTNodePtr = ptr<ParsableASTNode>;
class ASTNode;
using ASTNodePtr = ptr<ASTNode>;
class ASTTy;
using ASTTyPtr = ptr<ASTTy>;

class AnalyzerImpl {
public:
  AnalyzerImpl(CompilerSession *cs);
  void analyze(ParsableASTNodePtr &p);

private:
  ASTTyPtr get_ty(const ParsableASTNodePtr &p) const;
  ASTNodePtr try_convert_to_ast_node(const ParsableASTNodePtr &p) const;

  void analyze_member_access(ParsableASTNodePtr &p);
  void analyze_intrinsic(ParsableASTNodePtr &p);
  void resolve_ty(const ASTTyPtr &p) const;

private:
  CompilerSession *_cs = nullptr;
};

}

#endif //__TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
