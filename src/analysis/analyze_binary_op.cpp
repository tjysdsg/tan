#include "src/analysis/analyzer_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_node.h"

using namespace tanlang;

void AnalyzerImpl::analyze_assignment(const ParsableASTNodePtr &p) {
  ASTNodePtr np = ast_must_cast<ASTNode>(p);
  ASTNodePtr lhs = p->get_child_at<ASTNode>(0);
  ASTNodePtr rhs = p->get_child_at<ASTNode>(1);
  analyze(rhs);
  if (!lhs->_ty) { /// the type of lhs is not set, we deduce it
    lhs->_ty = copy_ty(rhs->_ty);
  }
  analyze(lhs);

  if (TypeSystem::CanImplicitCast(_cs, lhs->_ty, rhs->_ty) != 0) {
    report_error(p, "Cannot perform implicit type conversion");
  }
  np->_ty = lhs->_ty;
}
