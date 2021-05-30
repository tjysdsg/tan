#ifndef __TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
#include "base.h"
#include "src/analysis/ast_helper.h"

namespace tanlang {

AST_FWD_DECL(ASTBase);
AST_FWD_DECL(ASTType);
AST_FWD_DECL(BinaryOperator);
AST_FWD_DECL(MemberAccess);

class AnalyzerImpl {
public:
  AnalyzerImpl(CompilerSession *cs);
  void analyze(const ASTBasePtr &p);

private:
  void analyze_member_access(const MemberAccessPtr &p);
  void analyze_intrinsic(const ASTBasePtr &p);
  void analyze_string_literal(const ASTBasePtr &p);
  void analyze_char_literal(const ASTBasePtr &p);
  void analyze_integer_literal(const ASTBasePtr &p);
  void analyze_float_literal(const ASTBasePtr &p);
  void analyze_array_literal(const ASTBasePtr &p);
  void analyze_struct_decl(const ASTBasePtr &p);
  void analyze_func_decl(const ASTBasePtr &p);
  void analyze_func_call(const ASTBasePtr &p);
  void analyze_import(const ASTBasePtr &p);
  void analyze_parenthesis(const ASTBasePtr &p);
  void analyze_if(const ASTBasePtr &p);
  void analyze_assignment(const BinaryOperatorPtr &p);
  void analyze_bop(const ASTBasePtr &p);
  void analyze_uop(const ASTBasePtr &p);
  void analyze_id(const ASTBasePtr &p);
  void analyze_var_decl(const ASTBasePtr &p);
  void analyze_arg_decl(const ASTBasePtr &p);
  void resolve_ty(const ASTTypePtr &p) const;
  ASTTypePtr copy_ty(const ASTTypePtr &p) const;
  [[noreturn]] void report_error(const ASTBasePtr &p, const str &message);

private:
  CompilerSession *_cs = nullptr;
  ASTHelper _h;
};

}

#endif //__TAN_SRC_ANALYSIS_ANALYZER_IMPL_H__
