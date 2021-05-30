#include "analyzer_impl.h"
#include "src/ast/ast_type.h"
#include "compiler_session.h"
#include "src/ast/factory.h"
#include "src/ast/ast_base.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_member_access.h"
#include <fmt/core.h>

using namespace tanlang;

void AnalyzerImpl::analyze_member_access(const ASTBasePtr &p) {
  auto np = ast_must_cast<ASTNode>(p);

  auto lhs = p->get_child_at(0);
  auto pma = ast_must_cast<ASTMemberAccess>(p);

  if (pma->_access_type == MemberAccessType::MemberAccessDeref) { /// pointer dereference
    auto ty = _h.get_ty(lhs);
    TAN_ASSERT(ty->_is_ptr);
    ty = make_ptr<ASTTypepe>(*_h.get_contained_ty(ty));
    ty->_is_lvalue = true;
    np->_type = ty;
  } else if (pma->_access_type == MemberAccessType::MemberAccessBracket) {
    auto rhs = p->get_child_at(1);

    ASTTypepePtr ty = _h.get_ty(lhs);
    if (!ty->_is_ptr) {
      report_error(p, "Expect a pointer or an array");
    }

    ty = make_ptr<ASTType>(*_h.get_contained_ty(ty));
    ty->_is_lvalue = true;
    if (!ty) {
      report_error(p, "Unable to perform bracket access");
    }

    np->_type = ty;
    if (rhs->get_node_type() == ASTNodeType::NUM_LITERAL) {
      if (!_h.get_ty(rhs)->_is_int) {
        report_error(p, "Expect an integer specifying array size");
      }
      auto size = rhs->get_data<uint64_t>(); // underflow is ok
      if (_h.get_ty(rhs)->_is_array && size >= _h.get_ty(lhs)->_array_size) {
        report_error(p,
            fmt::format("Index {} out of bound, the array size is {}",
                std::to_string(size),
                std::to_string(_h.get_ty(lhs)->_array_size)));
      }
    }
  } else if (p->get_child_at(1)->get_node_type() == ASTNodeType::ID) { /// member variable or enum
    auto rhs = p->get_child_at(1);
    if (_h.get_ty(lhs)->_is_enum) {
      // TODO: Member access of enums
    } else {
      pma->_access_type = MemberAccessType::MemberAccessMemberVariable;
      if (!_h.get_ty(lhs)->_is_lvalue && !_h.get_ty(lhs)->_is_ptr) {
        report_error(p, "Invalid left-hand operand");
      }

      str m_name = rhs->get_data<str>();
      std::shared_ptr<ASTType> struct_ast = nullptr;
      /// auto dereference pointers
      if (_h.get_ty(lhs)->_is_ptr) {
        struct_ast = _cs->get_type(_h.get_contained_ty(_h.get_ty(lhs))->_type_name);
      } else {
        struct_ast = _cs->get_type(_h.get_ty(lhs)->_type_name);
      }
      TAN_ASSERT(struct_ast);
      pma->_access_idx = _h.get_struct_member_index(struct_ast, m_name);
      np->_type = make_ptr<ASTType>(*_h.get_struct_member_ty(struct_ast, pma->_access_idx));
      np->_type->_is_lvalue = true;
    }
  } else if (pma->_access_type == MemberAccessType::MemberAccessMemberFunction) { /// method call
    auto rhs = p->get_child_at(1);
    if (!_h.get_ty(lhs)->_is_lvalue && !_h.get_ty(lhs)->_is_ptr) {
      report_error(p, "Method calls require left-hand operand to be an lvalue or a pointer");
    }
    /// get address of the struct instance
    if (_h.get_ty(lhs)->_is_lvalue && !_h.get_ty(lhs)->_is_ptr) {
      ASTNodePtr tmp = ast_create_address_of(_cs, lhs);
      analyze(tmp);
      rhs->get_children().insert(rhs->get_children().begin(), tmp);
    } else {
      rhs->get_children().insert(rhs->get_children().begin(), lhs);
    }
    /// TODO: postpone analysis of FUNC_CALL until now
    analyze(rhs);
    np->_type = _h.get_ty(rhs);
  } else {
    report_error(p, "Invalid right-hand operand");
  }
}
