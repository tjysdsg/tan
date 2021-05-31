#include "analyzer_impl.h"
#include "src/ast/ast_type.h"
#include "src/ast/expr.h"
#include "src/ast/decl.h"
#include "src/ast/ast_struct.h"
#include "src/analysis/type_system.h"
#include "compiler_session.h"
#include <fmt/core.h>

using namespace tanlang;

void AnalyzerImpl::analyze_member_access(const MemberAccessPtr &p) {
  ptr<Expr> lhs = p->get_lhs();
  ptr<Expr> rhs = p->get_rhs();

  if (p->_access_type == MemberAccess::MemberAccessDeref) { /// pointer dereference
    auto ty = lhs->get_type();
    TAN_ASSERT(ty->_is_ptr);
    ty = copy_ty(_h.get_contained_ty(ty));
    ty->_is_lvalue = true;
    p->set_type(ty);
  } else if (p->_access_type == MemberAccess::MemberAccessBracket) {
    auto ty = lhs->get_type();
    if (!ty->_is_ptr) {
      report_error(p, "Expect a pointer or an array");
    }

    ty = copy_ty(_h.get_contained_ty(ty));
    ty->_is_lvalue = true;
    if (!ty) {
      report_error(p, "Unable to perform bracket access");
    }

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
    } else {
      report_error(p, "Expect an integer specifying array size");
    }
  } else if (rhs->get_node_type() == ASTNodeType::ID) { /// member variable or enum
    if (lhs->get_type()->_is_enum) {
      // TODO: Member access of enums
      TAN_ASSERT(false);
    } else {
      p->_access_type = MemberAccess::MemberAccessMemberVariable;
      if (!lhs->get_type()->_is_lvalue && !lhs->get_type()->_is_ptr) {
        report_error(p, "Invalid left-hand operand");
      }

      str m_name = ast_must_cast<Identifier>(rhs)->get_name();
      ASTTypePtr _struct_ast = nullptr;
      ASTStructPtr struct_ast = nullptr;
      /// auto dereference pointers
      if (lhs->get_type()->_is_ptr) {
        _struct_ast = _cs->get_type(_h.get_contained_ty(lhs->get_type())->_type_name);
      } else {
        _struct_ast = _cs->get_type(lhs->get_type()->_type_name);
      }
      if (!(struct_ast = ast_cast<ASTStruct>(_struct_ast))) {
        report_error(_struct_ast, "Expect a struct type");
      }
      p->_access_idx = _h.get_struct_member_index(struct_ast, m_name);
      auto ty = copy_ty(_h.get_struct_member_ty(struct_ast, p->_access_idx));
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
      ptr<Expr> tmp = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF, lhs);
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
