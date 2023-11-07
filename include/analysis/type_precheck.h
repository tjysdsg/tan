#ifndef __TAN_ANALYSIS_TYPE_PRE_CHECK_H__
#define __TAN_ANALYSIS_TYPE_PRE_CHECK_H__
#include "base.h"
#include "analysis/analysis_action.h"
#include "common/dependency_graph.h"
#include "ast/package.h"

namespace tanlang {

class Decl;
class Expr;
class Type;
class Program;
class TokenizedSourceFile;
class ASTBase;

/**
 * \brief Perform preliminary type checking. We try our best to resolve types,
 *        and remember those that cannot be fully resolved plus their symbol dependencies.
 * \details This class only operates on top-level declarations, such as functions, structs, ...
 *          And it expects all input AST nodes to contain a non-empty type.
 */
class TypePrecheck : public SemanticAnalysisAction<TypePrecheck, Package *, void> {
public:
  void run_impl(Package *cu);

  void default_visit(ASTBase *p) override;

private:
  /**
   * \brief Try to resolve a type by its name.
   *        By resolve we mean figuring out all required information related to bit size, alignment, etc.
   *        If this is currently not possible, it will be stored in a dependency graph and analyzed again in later
   *        stages.
   *
   * \param p The type
   * \param node The AST node requires \p p to be resolved.
   * \return The resolved type. Returns \p p as is if failed.
   */
  Type *check_type_ref(Type *p, ASTBase *node);

  /**
   * \brief Try to resolve a type. Trivial if it's plain old data.
   *        Most of the work is done for functions, structs, typedefs, etc.
   *
   * \param p The type
   * \param node The AST node requires p to be resolved.
   * \return A resolved type, could be a different Type instance than \p p.
   * \sa check_type_ref
   */
  Type *check_type(Type *p, ASTBase *node);

public:
  DECLARE_AST_VISITOR_IMPL(VarDecl);
  DECLARE_AST_VISITOR_IMPL(ArgDecl);
  DECLARE_AST_VISITOR_IMPL(Assignment);
  DECLARE_AST_VISITOR_IMPL(FunctionDecl);
  DECLARE_AST_VISITOR_IMPL(Import);
  DECLARE_AST_VISITOR_IMPL(Intrinsic);
  DECLARE_AST_VISITOR_IMPL(StructDecl);

private:
  Package *_package = nullptr;
};

} // namespace tanlang

#endif // __TAN_ANALYSIS_TYPE_PRE_CHECK_H__
