#ifndef __TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
#include "base.h"
#include "src/analysis/ast_helper.h"
#include "src/ast/fwd.h"

namespace tanlang {

class AnalyzerImpl {
public:
  AnalyzerImpl(CompilerSession *cs);
  void analyze(ASTBase *p);

private:
  void analyze_stmt(ASTBase *p);
  void analyze_member_access(MemberAccess *p);
  void analyze_intrinsic(ASTBase *p);
  void analyze_string_literal(ASTBase *p);
  void analyze_char_literal(ASTBase *p);
  void analyze_integer_literal(ASTBase *p);
  void analyze_float_literal(ASTBase *p);
  void analyze_array_literal(ASTBase *p);
  void analyze_struct_decl(ASTBase *p);
  void analyze_func_decl(ASTBase *p);
  void analyze_func_call(ASTBase *p);
  void analyze_import(ASTBase *p);
  void analyze_ret(ASTBase *p);
  void analyze_parenthesis(ASTBase *p);
  void analyze_if(ASTBase *p);
  void analyze_assignment(Assignment *p);
  void analyze_cast(Cast *p);
  void analyze_bop(ASTBase *p);
  void analyze_uop(ASTBase *p);
  void analyze_id(ASTBase *p);
  void analyze_var_decl(ASTBase *p);
  void analyze_arg_decl(ASTBase *p);
  void analyze_bop_or_uop(ASTBase *p);
  void resolve_ty(ASTType *p) const;
  ASTType *copy_ty(ASTType *p) const;
  [[noreturn]] void report_error(ASTBase *p, const str &message);

private:
  CompilerSession *_cs = nullptr;
  ASTHelper _h;
};

}

#endif //__TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
