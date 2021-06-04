#include "analyzer_impl.h"
#include "src/ast/ast_type.h"
#include "src/ast/expr.h"
#include "src/ast/decl.h"
#include "src/analysis/type_system.h"
#include "compiler_session.h"
#include <fmt/core.h>

using namespace tanlang;

void AnalyzerImpl::analyze_member_access(MemberAccess *p) {
  Expr *lhs = p->get_lhs();
  analyze(lhs);
  Expr *rhs = p->get_rhs();

  if (p->_access_type == MemberAccess::MemberAccessDeref) { /// pointer dereference
    analyze(rhs);
    auto ty = lhs->get_type();
    TAN_ASSERT(ty->_is_ptr);
    ty = copy_ty(_h.get_contained_ty(ty));
    ty->_is_lvalue = true;
    p->set_type(ty);
  } else if (p->_access_type == MemberAccess::MemberAccessBracket) {
    analyze(rhs);
    auto ty = lhs->get_type();
    if (!ty->_is_ptr) {
      report_error(p, "Expect a pointer or an array");
    }

    ty = copy_ty(_h.get_contained_ty(ty));
    ty->_is_lvalue = true;

    p->set_type(ty);
    if (rhs->get_node_type() == ASTNodeType::INTEGER_LITERAL) {
      auto size_node = ast_must_cast<IntegerLiteral>(rhs);
      auto size = size_node->get_value(); // underflow is ok
      if (rhs->get_type()->_is_array && size >= lhs->get_type()->_array_size) {
        report_error(p,
            fmt::format("Index {} out of bound, the array size is {}",
                std::to_string(size),
                std::to_string(lhs->get_type()->_array_size)));
      }
    }
  } else if (rhs->get_node_type() == ASTNodeType::ID) { /// member variable or enum
    analyze(rhs);
    if (lhs->get_type()->_is_enum) {
      // TODO: Member access of enums
      TAN_ASSERT(false);
    } else {
      p->_access_type = MemberAccess::MemberAccessMemberVariable;
      if (!lhs->get_type()->_is_lvalue && !lhs->get_type()->_is_ptr) {
        report_error(p, "Invalid left-hand operand");
      }

      str m_name = ast_must_cast<Identifier>(rhs)->get_name();
      ASTType *struct_ast = nullptr;
      /// auto dereference pointers
      if (lhs->get_type()->_is_ptr) {
        struct_ast = _h.get_contained_ty(lhs->get_type());
      } else {
        struct_ast = lhs->get_type();
      }
      if (struct_ast->_tyty != Ty::STRUCT) {
        report_error(lhs, "Expect a struct type");
      }

      StructDecl *struct_decl = ast_must_cast<StructDecl>(_cs->get_type_decl(struct_ast->_type_name));
      p->_access_idx = struct_decl->get_struct_member_index(m_name);
      auto ty = copy_ty(struct_decl->get_struct_member_ty(p->_access_idx));
      ty->_is_lvalue = true;
      p->set_type(ty);
    }
  } else if (p->_access_type == MemberAccess::MemberAccessMemberFunction) { /// method call
    if (!lhs->get_type()->_is_lvalue && !lhs->get_type()->_is_ptr) {
      report_error(p, "Method calls require left-hand operand to be an lvalue or a pointer");
    }
    auto func_call = ast_cast<FunctionCall>(rhs);
    if (!func_call) {
      report_error(rhs, "Expect a function call");
    }

    /// get address of the struct instance
    if (lhs->get_type()->_is_lvalue && !lhs->get_type()->_is_ptr) {
      Expr *tmp = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF, lhs);
      tmp->_start_index = lhs->_start_index;
      tmp->_end_index = lhs->_end_index;
      tmp->set_token(lhs->get_token());
      analyze(tmp);

      func_call->_args.push_back(tmp);
    } else {
      func_call->_args.push_back(lhs);
    }

    /// postpone analysis of FUNC_CALL until now
    analyze(rhs);
    p->set_type(copy_ty(rhs->get_type()));
  } else {
    report_error(p, "Invalid right-hand operand");
  }
}
