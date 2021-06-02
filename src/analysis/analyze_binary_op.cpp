#include "src/ast/expr.h"
#include "src/ast/decl.h"
#include "src/ast/ast_type.h"
#include "src/analysis/analyzer_impl.h"
#include "src/analysis/type_system.h"
#include "compiler_session.h"
#include "src/common.h"

using namespace tanlang;

void AnalyzerImpl::analyze_cast(const CastPtr &p) {
  ptr<Expr> lhs = p->get_lhs();
  ASTBasePtr rhs = p->get_rhs();
  analyze(lhs);
  analyze(rhs);

  ASTTypePtr ty = nullptr;
  switch (rhs->get_node_type()) {
    case ASTNodeType::ID:
      ty = _cs->get_type(ast_must_cast<Identifier>(rhs)->get_name());
      if (!ty) {
        report_error(rhs, "Unknown type");
      }
      break;
    case ASTNodeType::TY:
      ty = ast_must_cast<ASTType>(rhs);
      break;
    default:
      report_error(lhs, "Invalid right-hand operand");
      break;
  }

  ty = copy_ty(ty);
  ty->_is_lvalue = lhs->get_type()->_is_lvalue;
  p->set_type(ty);
  // FIXME: check if can explicit cast
  // if (TypeSystem::CanImplicitCast(_cs, np->_type, _h.get_ty(p->get_child_at(0))) != 0) {
  //   report_error(p, "Cannot perform implicit type conversion");
  // }
}

void AnalyzerImpl::analyze_assignment(const AssignmentPtr &p) {
  ptr<Expr> rhs = p->get_rhs();
  analyze(rhs);

  auto lhs = p->get_lhs();
  ASTTypePtr lhs_type = nullptr;
  switch (lhs->get_node_type()) {
    case ASTNodeType::ID:
      analyze(lhs);
      lhs_type = ast_must_cast<Identifier>(lhs)->get_type();
      break;
    case ASTNodeType::STRUCT_DECL:
    case ASTNodeType::VAR_DECL:
    case ASTNodeType::ARG_DECL:
    case ASTNodeType::ENUM_DECL:
      lhs_type = ast_must_cast<Decl>(lhs)->get_type();
      break;
    default:
      report_error(lhs, "Invalid left-hand operand");
  }

  /// if the type of lhs is not set, we deduce it
  /// NOTE: we only allow type deduction for declarations
  if (!lhs_type) {
    lhs_type = copy_ty(rhs->get_type());

    /// set type of lhs
    switch (lhs->get_node_type()) {
      case ASTNodeType::STRUCT_DECL:
      case ASTNodeType::VAR_DECL:
      case ASTNodeType::ARG_DECL:
      case ASTNodeType::ENUM_DECL:
        ast_must_cast<Decl>(lhs)->set_type(lhs_type);
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
    /// already analyzed if lhs is identifier, so put this here to
    /// avoid analyzing twice
    analyze(lhs);
  }

  if (TypeSystem::CanImplicitCast(_cs, lhs_type, rhs->get_type()) != 0) {
    report_error(p, "Cannot perform implicit type conversion");
  }
  p->set_type(lhs_type);
}

void AnalyzerImpl::analyze_bop(const ASTBasePtr &_p) {
  auto p = ast_must_cast<BinaryOperator>(_p);
  ptr<Expr> lhs = p->get_lhs();
  ptr<Expr> rhs = p->get_rhs();

  /// NOTE: do not analyze lhs and rhs just yet, because analyze_assignment
  /// and analyze_member_access have their own ways of analyzing

  switch (p->get_op()) {
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
    case BinaryOpKind::MEMBER_ACCESS:
      analyze_member_access(ast_must_cast<MemberAccess>(p));
      break;
    default:
      TAN_ASSERT(false);
      break;
  }
}
