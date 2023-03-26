#ifndef __TAN_ANALYSIS_TYPE_CHECK_H__
#define __TAN_ANALYSIS_TYPE_CHECK_H__
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

class TypeCheck : public AnalysisAction<TypeCheck, Program *, void> {
public:
  TypeCheck() = delete;
  explicit TypeCheck(SourceManager *sm);

  void run_impl(Program *) { // TODO
  }

  void stage2(Program *p, const vector<ASTBase *> &sorted_top_level_decls);

private:
  SourceManager *_sm = nullptr;

private:
  /**
   * \brief Check whether it's legal to implicitly convert from type `from` to type `to`
   *        See TYPE_CASTING.md for specifications.
   * \param from Source type.
   * \param to Destination type.
   */
  static bool CanImplicitlyConvert(Type *from, Type *to);

  /**
   * \brief Find out which one of the two input types of a binary operation should operands promote to.
   *        See TYPE_CASTING.md for specifications.
   * \return Guaranteed to be one of `t1` and `t2`, or nullptr if cannot find a legal promotion.
   */
  static Type *ImplicitTypePromote(Type *t1, Type *t2);

  Cast *create_implicit_conversion(Expr *from, Type *to);

  /**
   * \brief Find the type that operands of a BOP should promote to, and add a Cast node to the AST.
   *        Raise an error if can't find a valid type promotion.
   * \note This could modify the lhs or rhs of `bop`, make sure to update the references to any of them after calling.
   * \return The promoted type.
   */
  Type *auto_promote_bop_operand_types(BinaryOperator *bop);

  FunctionDecl *search_function_callee(FunctionCall *p);

  /**
   * \brief Resolve a type reference.
   * \return Non-null
   */
  Type *resolve_type_ref(Type *p, SrcLoc loc);

  /**
   * \brief Resolve a type.
   * \return Non-null
   */
  Type *resolve_type(Type *p, SrcLoc loc);

public:
  DECLARE_AST_VISITOR_IMPL(Program);
  DECLARE_AST_VISITOR_IMPL(Identifier);
  DECLARE_AST_VISITOR_IMPL(Parenthesis);
  DECLARE_AST_VISITOR_IMPL(If);
  DECLARE_AST_VISITOR_IMPL(VarDecl);
  DECLARE_AST_VISITOR_IMPL(ArgDecl);
  DECLARE_AST_VISITOR_IMPL(Return);
  DECLARE_AST_VISITOR_IMPL(CompoundStmt);
  DECLARE_AST_VISITOR_IMPL(BinaryOrUnary);
  DECLARE_AST_VISITOR_IMPL(BinaryOperator);
  DECLARE_AST_VISITOR_IMPL(UnaryOperator);
  DECLARE_AST_VISITOR_IMPL(Cast);
  DECLARE_AST_VISITOR_IMPL(Assignment);
  DECLARE_AST_VISITOR_IMPL(FunctionCall);
  DECLARE_AST_VISITOR_IMPL(FunctionDecl);
  // DECLARE_AST_VISITOR_IMPL(Import);
  DECLARE_AST_VISITOR_IMPL(Intrinsic);
  DECLARE_AST_VISITOR_IMPL(ArrayLiteral);
  DECLARE_AST_VISITOR_IMPL(CharLiteral);
  DECLARE_AST_VISITOR_IMPL(BoolLiteral);
  DECLARE_AST_VISITOR_IMPL(IntegerLiteral);
  DECLARE_AST_VISITOR_IMPL(FloatLiteral);
  DECLARE_AST_VISITOR_IMPL(StringLiteral);
  DECLARE_AST_VISITOR_IMPL(MemberAccess);
  DECLARE_AST_VISITOR_IMPL(StructDecl);
  DECLARE_AST_VISITOR_IMPL(Loop);
  DECLARE_AST_VISITOR_IMPL(BreakContinue);

private:
  void analyze_func_decl_prototype(ASTBase *_p);

  void analyze_func_body(ASTBase *_p);

  void analyze_intrinsic_func_call(Intrinsic *p, FunctionCall *func_call);

  /// search for the intrinsic type
  void find_and_assign_intrinsic_type(Intrinsic *p, const str &name);

  void analyze_member_func_call(MemberAccess *p, Expr *lhs, FunctionCall *rhs);

  void analyze_bracket_access(MemberAccess *p, Expr *lhs, Expr *rhs);

  void analyze_member_access_member_variable(MemberAccess *p, Expr *lhs, Expr *rhs);
};

} // namespace tanlang

#endif // __TAN_ANALYSIS_TYPE_CHECK_H__
