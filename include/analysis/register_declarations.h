#ifndef __TAN_ANALYSIS_REGISTER_DECLARATIONS_H__
#define __TAN_ANALYSIS_REGISTER_DECLARATIONS_H__

#include "analysis/analysis_action.h"

namespace tanlang {

/**
 * \brief Register all declarations (including local) in the corresponding scopes.
 *        Run this stage early to easily obtain a list of top-level declarations from each source file.
 */
class RegisterDeclarations : public SemanticAnalysisAction<RegisterDeclarations, Program *, void> {
public:
  void run_impl(Program *p);

  DECLARE_AST_VISITOR_IMPL(Program);
  // DECLARE_AST_VISITOR_IMPL(Identifier);
  DECLARE_AST_VISITOR_IMPL(Parenthesis);
  DECLARE_AST_VISITOR_IMPL(If);
  DECLARE_AST_VISITOR_IMPL(VarDecl);
  DECLARE_AST_VISITOR_IMPL(ArgDecl);
  DECLARE_AST_VISITOR_IMPL(CompoundStmt);
  DECLARE_AST_VISITOR_IMPL(BinaryOrUnary);
  DECLARE_AST_VISITOR_IMPL(BinaryOperator);
  DECLARE_AST_VISITOR_IMPL(UnaryOperator);
  DECLARE_AST_VISITOR_IMPL(Assignment);
  DECLARE_AST_VISITOR_IMPL(FunctionDecl);
  DECLARE_AST_VISITOR_IMPL(Intrinsic);
  DECLARE_AST_VISITOR_IMPL(StructDecl);
  DECLARE_AST_VISITOR_IMPL(Loop);
};

} // namespace tanlang

#endif //__TAN_ANALYSIS_REGISTER_DECLARATIONS_H__
