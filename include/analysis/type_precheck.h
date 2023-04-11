#ifndef __TAN_ANALYSIS_TYPE_PRE_CHECK_H__
#define __TAN_ANALYSIS_TYPE_PRE_CHECK_H__
#include "base.h"
#include "analysis/analysis_action.h"
#include "common/dependency_graph.h"

namespace tanlang {

class Decl;
class Expr;
class Type;
class Program;
class SourceManager;
class ASTBase;

class TypePrecheck : public SingleUnitAnalysisAction<TypePrecheck, void> {
public:
  void run_impl(CompilationUnit *cu);

  void default_visit(ASTBase *p) override;

private:
  /**
   * \brief Resolve type reference.
   *        In non-strict mode, this adds a dependency from \p node to the referred declaration D
   *        if D doesn't have a resolved type yet.
   *        In strict mode, an error is raised.
   * \return The referred type if successfully resolved. Return \p p as is if failed.
   */
  Type *check_type_ref(Type *p, SrcLoc loc, ASTBase *node);

  /**
   * \brief Resolve a type. If \p is a type reference, we find out the type associated with the typename.
   * \note Returned pointer can be different from \p p.
   */
  Type *check_type(Type *p, SrcLoc loc, ASTBase *node);

public:
  // DECLARE_AST_VISITOR_IMPL(Program);
  // DECLARE_AST_VISITOR_IMPL(Identifier);
  // DECLARE_AST_VISITOR_IMPL(Parenthesis);
  // DECLARE_AST_VISITOR_IMPL(If);
  DECLARE_AST_VISITOR_IMPL(VarDecl);
  DECLARE_AST_VISITOR_IMPL(ArgDecl);
  // DECLARE_AST_VISITOR_IMPL(Return);
  // DECLARE_AST_VISITOR_IMPL(CompoundStmt);
  // DECLARE_AST_VISITOR_IMPL(BinaryOrUnary);
  // DECLARE_AST_VISITOR_IMPL(BinaryOperator);
  // DECLARE_AST_VISITOR_IMPL(UnaryOperator);
  // DECLARE_AST_VISITOR_IMPL(Cast);
  // DECLARE_AST_VISITOR_IMPL(Assignment);
  // DECLARE_AST_VISITOR_IMPL(FunctionCall);
  DECLARE_AST_VISITOR_IMPL(FunctionDecl);
  DECLARE_AST_VISITOR_IMPL(Import);
  DECLARE_AST_VISITOR_IMPL(Intrinsic);
  // DECLARE_AST_VISITOR_IMPL(ArrayLiteral);
  // DECLARE_AST_VISITOR_IMPL(CharLiteral);
  // DECLARE_AST_VISITOR_IMPL(BoolLiteral);
  // DECLARE_AST_VISITOR_IMPL(IntegerLiteral);
  // DECLARE_AST_VISITOR_IMPL(FloatLiteral);
  // DECLARE_AST_VISITOR_IMPL(StringLiteral);
  // DECLARE_AST_VISITOR_IMPL(MemberAccess);
  DECLARE_AST_VISITOR_IMPL(StructDecl);
  // DECLARE_AST_VISITOR_IMPL(Loop);
  // DECLARE_AST_VISITOR_IMPL(BreakContinue);

private:
  CompilationUnit *_cu = nullptr;
};

} // namespace tanlang

#endif // __TAN_ANALYSIS_TYPE_PRE_CHECK_H__
