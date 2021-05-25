#include "analyzer_impl.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "src/ast/factory.h"
#include "src/ast/parsable_ast_node.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_member_access.h"

using namespace tanlang;

void AnalyzerImpl::analyze_member_access(ParsableASTNodePtr &p) {
  auto np = ast_must_cast<ASTNode>(p);

  auto lhs = p->get_child_at(0);
  auto pma = ast_must_cast<ASTMemberAccess>(p);

  if (pma->_access_type == MemberAccessType::MemberAccessDeref) { /// pointer dereference
    auto ty = _h.get_ty(lhs);
    TAN_ASSERT(ty->_is_ptr);
    ty = make_ptr<ASTTy>(*_h.get_contained_ty(ty));
    ty->_is_lvalue = true;
    np->_ty = ty;
  } else if (pma->_access_type == MemberAccessType::MemberAccessBracket) {
    auto rhs = p->get_child_at(1);

    ASTTyPtr ty = _h.get_ty(lhs);
    if (!ty->_is_ptr) { report_error(_cs, p, "Expect a pointer or an array"); }

    ty = make_ptr<ASTTy>(*_h.get_contained_ty(ty));
    ty->_is_lvalue = true;
    if (!ty) { report_error(_cs, p, "Unable to perform bracket access"); }

    np->_ty = ty;
    if (rhs->get_node_type() == ASTType::NUM_LITERAL) {
      if (!_h.get_ty(rhs)->_is_int) { report_error(_cs, p, "Expect an integer specifying array size"); }
      auto size = rhs->get_data<uint64_t>(); // underflow is ok
      if (_h.get_ty(rhs)->_is_array && size >= _h.get_ty(lhs)->_array_size) {
        report_error(_cs,
            p,
            "Index " + std::to_string(size) + " out of bound, the array size is "
                + std::to_string(_h.get_ty(lhs)->_array_size));
      }
    }
  } else if (p->get_child_at(1)->get_node_type() == ASTType::ID) { /// member variable or enum
    auto rhs = p->get_child_at(1);
    if (_h.get_ty(lhs)->_is_enum) {
      // TODO: Member access of enums
    } else {
      pma->_access_type = MemberAccessType::MemberAccessMemberVariable;
      if (!_h.get_ty(lhs)->_is_lvalue && !_h.get_ty(lhs)->_is_ptr) {
        report_error(_cs, p, "Invalid left-hand operand");
      }

      str m_name = rhs->get_data<str>();
      std::shared_ptr<ASTTy> struct_ast = nullptr;
      /// auto dereference pointers
      if (_h.get_ty(lhs)->_is_ptr) {
        struct_ast = _cs->get_type(_h.get_contained_ty(_h.get_ty(lhs))->_type_name);
      } else {
        struct_ast = _cs->get_type(_h.get_ty(lhs)->_type_name);
      }
      TAN_ASSERT(struct_ast);
      pma->_access_idx = _h.get_struct_member_index(struct_ast, m_name);
      np->_ty = make_ptr<ASTTy>(*_h.get_struct_member_ty(struct_ast, pma->_access_idx));
      np->_ty->_is_lvalue = true;
    }
  } else if (pma->_access_type == MemberAccessType::MemberAccessMemberFunction) { /// method call
    auto rhs = p->get_child_at(1);
    if (!_h.get_ty(lhs)->_is_lvalue && !_h.get_ty(lhs)->_is_ptr) {
      report_error(_cs, p, "Method calls require left-hand operand to be an lvalue or a pointer");
    }
    /// get address of the struct instance
    if (_h.get_ty(lhs)->_is_lvalue && !_h.get_ty(lhs)->_is_ptr) {
      auto tmp = ast_create_address_of(_cs, lhs);
      analyze(tmp);
      rhs->get_children().insert(rhs->get_children().begin(), tmp);
    } else {
      rhs->get_children().insert(rhs->get_children().begin(), lhs);
    }
    /// TODO: postpone analysis of FUNC_CALL until now
    analyze(rhs);
    np->_ty = _h.get_ty(rhs);
  } else {
    report_error(_cs, p, "Invalid right-hand operand");
  }
}
