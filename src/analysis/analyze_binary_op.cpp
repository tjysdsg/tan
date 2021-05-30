#include "src/ast/expr.h"
#include "src/ast/ast_type.h"
#include "src/analysis/analyzer_impl.h"
#include "src/analysis/type_system.h"

using namespace tanlang;

void AnalyzerImpl::analyze_assignment(const BinaryOperatorPtr &p) {
  ptr<Expr> lhs = p->get_lhs();
  ptr<Expr> rhs = p->get_rhs();
  analyze(rhs);
  if (!lhs->get_type()) { /// the type of lhs is not set, we deduce it
    lhs->set_type(copy_ty(rhs->get_type()));
  }
  analyze(lhs);

  if (TypeSystem::CanImplicitCast(_cs, lhs->get_type(), rhs->get_type()) != 0) {
    report_error(p, "Cannot perform implicit type conversion");
  }
  p->set_type(lhs->get_type());
}

void AnalyzerImpl::analyze_bop(const ASTBasePtr &_p) {
  auto p = ast_must_cast<BinaryOperator>(_p);
  ptr<Expr> lhs = p->get_lhs();
  ptr<Expr> rhs = p->get_rhs();

  /// NOTE: do not analyze lhs and rhs just yet, because analyze_assignment
  /// and analyze_member_access have their own ways of analyzing

  switch (p->get_op()) {
    case BinaryOpKind::ASSIGN:
      analyze_assignment(p);
      break;
    case BinaryOpKind::SUM:
    case BinaryOpKind::SUBTRACT:
    case BinaryOpKind::MULTIPLY:
    case BinaryOpKind::DIVIDE:
    case BinaryOpKind::MOD: {
      analyze(lhs);
      analyze(rhs);

      int i = TypeSystem::CanImplicitCast(_cs, lhs->get_type(), rhs->get_type());
      if (i == -1) {
        report_error(p, "Cannot perform implicit type conversion");
      }

      size_t dominant_idx = static_cast<size_t>(i);
      p->set_dominant_idx(dominant_idx);
      ASTTypePtr ty = copy_ty(dominant_idx == 0 ? lhs->get_type() : rhs->get_type());
      ty->_is_lvalue = false;
      p->set_type(ty);
      break;
    }
    case BinaryOpKind::BAND:
    case BinaryOpKind::LAND:
    case BinaryOpKind::BOR:
    case BinaryOpKind::LOR:
    case BinaryOpKind::XOR:
      // TODO: implement the analysis of the above operators
      TAN_ASSERT(false);
      break;
    case BinaryOpKind::GT:
    case BinaryOpKind::GE:
    case BinaryOpKind::LT:
    case BinaryOpKind::LE:
    case BinaryOpKind::EQ:
    case BinaryOpKind::NE:
      analyze(lhs);
      analyze(rhs);

      p->set_type(ASTType::Create(_cs, Ty::BOOL));
      break;
    case BinaryOpKind::CAST: {
      analyze(lhs);
      analyze(rhs);

      auto ty = copy_ty(rhs->get_type());
      ty->_is_lvalue = lhs->get_type()->_is_lvalue;
      p->set_type(ty);
      // FIXME: check if can explicit cast
      // if (TypeSystem::CanImplicitCast(_cs, np->_type, _h.get_ty(p->get_child_at(0))) != 0) {
      //   report_error(p, "Cannot perform implicit type conversion");
      // }
      break;
    }
    case BinaryOpKind::MEMBER_ACCESS:
      analyze_member_access(p);
      break;
    default:
      TAN_ASSERT(false);
      break;
  }
}
