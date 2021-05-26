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
  void analyze(const ParsableASTNodePtr &p);

private:
  void analyze_member_access(const ParsableASTNodePtr &p);
  void analyze_intrinsic(const ParsableASTNodePtr &p);
  void analyze_char_literal(const ParsableASTNodePtr &p);
  void analyze_num_literal(const ParsableASTNodePtr &p);
  void analyze_array_literal(const ParsableASTNodePtr &p);
  void analyze_struct(const ParsableASTNodePtr &p);
  void analyze_func_decl(const ParsableASTNodePtr &p);
  void analyze_func_call(const ParsableASTNodePtr &p);
  void analyze_import(const ParsableASTNodePtr &p);
  void resolve_ty(const ASTTyPtr &p) const;
  ASTTyPtr copy_ty(const ASTTyPtr &p) const;
  [[noreturn]] void report_error(const ParsableASTNodePtr &p, const str &message);

private:
  CompilerSession *_cs = nullptr;
  ASTHelper _h;
};

}

#endif //__TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
