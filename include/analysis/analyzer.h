#ifndef __TAN_SRC_ANALYSIS_ANALYZER_H__
#define __TAN_SRC_ANALYSIS_ANALYZER_H__
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

class Analyzer : public AnalysisAction<Analyzer> {
public:
  Analyzer() = delete;
  explicit Analyzer(SourceManager *sm) : _sm(sm) {}

  void run_impl(Program *p) {
    // TODO: decouple stage 1 and stage 2
  }

  void stage1(Program *p);

  void stage2(Program *p, const vector<ASTBase *> &sorted_top_level_decls);

  vector<ASTBase *> sorted_unresolved_symbols() const;

private:
  SourceManager *_sm = nullptr;
  bool _strict = false;

  /// \brief Store unresolved symbols during non-strict parsing
  DependencyGraph _unresolved_symbols{};

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

  [[noreturn]] void error(ASTBase *p, const str &message);
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
   * \brief Resolve type reference.
   *        In non-strict mode, this adds a dependency from \p node to the referred declaration D
   *        if D doesn't have a resolved type yet.
   *        In strict mode, an error is raised.
   * \return The referred type if successfully resolved. Return \p p as is if failed.
   */
  Type *resolve_type_ref(Type *p, SrcLoc loc, ASTBase *node);

  /**
   * \brief Resolve a type. If \p is a type reference, we find out the type associated with the typename.
   * \note Returned pointer can be different from \p p.
   */
  Type *resolve_type(Type *p, SrcLoc loc, ASTBase *node);

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
  void add_decls_from_import(ASTBase *_p);

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

#endif //__TAN_SRC_ANALYSIS_ANALYZER_H__
