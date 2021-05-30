#include "src/analysis/analyzer_impl.h"
#include "src/analysis/type_system.h"

using namespace tanlang;

void AnalyzerImpl::analyze_assignment(const ASTBasePtr &p) {
  ASTNodePtr np = ast_must_cast<ASTNode>(p);
  ASTNodePtr lhs = p->get_child_at<ASTNode>(0);
  ASTNodePtr rhs = p->get_child_at<ASTNode>(1);
  analyze(rhs);
  if (!lhs->_type) { /// the type of lhs is not set, we deduce it
    lhs->_type = copy_ty(rhs->_type);
  }
  analyze(lhs);

  if (TypeSystem::CanImplicitCast(_cs, lhs->_type, rhs->_type) != 0) {
    report_error(p, "Cannot perform implicit type conversion");
  }
  np->_type = lhs->_type;
}
