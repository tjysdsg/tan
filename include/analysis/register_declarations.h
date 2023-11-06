#ifndef __TAN_ANALYSIS_REGISTER_DECLARATIONS_H__
#define __TAN_ANALYSIS_REGISTER_DECLARATIONS_H__

#include "analysis/analysis_action.h"

namespace tanlang {

class RegisterDeclarations : public SemanticAnalysisAction<RegisterDeclarations, CompilationUnit *, void> {
public:
  void run_impl(CompilationUnit *cu);

  DECLARE_AST_VISITOR_IMPL(Program);
  // DECLARE_AST_VISITOR_IMPL(Identifier);
  DECLARE_AST_VISITOR_IMPL(Parenthesis);
  DECLARE_AST_VISITOR_IMPL(If);
  DECLARE_AST_VISITOR_IMPL(VarDecl);
  DECLARE_AST_VISITOR_IMPL(ArgDecl);
  // DECLARE_AST_VISITOR_IMPL(Return);
  DECLARE_AST_VISITOR_IMPL(CompoundStmt);
  DECLARE_AST_VISITOR_IMPL(BinaryOrUnary);
  DECLARE_AST_VISITOR_IMPL(BinaryOperator);
  DECLARE_AST_VISITOR_IMPL(UnaryOperator);
  // DECLARE_AST_VISITOR_IMPL(Cast);
  DECLARE_AST_VISITOR_IMPL(Assignment);
  // DECLARE_AST_VISITOR_IMPL(FunctionCall);
  DECLARE_AST_VISITOR_IMPL(FunctionDecl);
  DECLARE_AST_VISITOR_IMPL(Intrinsic);
  // DECLARE_AST_VISITOR_IMPL(ArrayLiteral);
  // DECLARE_AST_VISITOR_IMPL(CharLiteral);
  // DECLARE_AST_VISITOR_IMPL(BoolLiteral);
  // DECLARE_AST_VISITOR_IMPL(IntegerLiteral);
  // DECLARE_AST_VISITOR_IMPL(FloatLiteral);
  // DECLARE_AST_VISITOR_IMPL(StringLiteral);
  // DECLARE_AST_VISITOR_IMPL(MemberAccess);
  DECLARE_AST_VISITOR_IMPL(StructDecl);
  DECLARE_AST_VISITOR_IMPL(Loop);
  // DECLARE_AST_VISITOR_IMPL(BreakContinue);
};

} // namespace tanlang

#endif //__TAN_ANALYSIS_REGISTER_DECLARATIONS_H__
