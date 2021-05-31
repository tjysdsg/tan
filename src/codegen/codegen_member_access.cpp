#include "src/codegen/code_generator_impl.h"
#include "src/llvm_include.h"
#include "src/ast/ast_type.h"
#include "src/ast/expr.h"
#include "compiler_session.h"

using namespace tanlang;

Value *CodeGeneratorImpl::codegen_member_access(const MemberAccessPtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();

  auto *from = codegen(lhs);
  Value *ret;
  switch (p->_access_type) {
    case MemberAccess::MemberAccessBracket: {
      if (lhs->get_type()->_is_lvalue) { from = builder->CreateLoad(from); }
      auto *rhs_val = codegen(rhs);
      if (rhs->get_type()->_is_lvalue) { rhs_val = builder->CreateLoad(rhs_val); }
      ret = builder->CreateGEP(from, rhs_val, "bracket_access");
      break;
    }
    case MemberAccess::MemberAccessMemberVariable: {
      if (lhs->get_type()->_is_lvalue && lhs->get_type()->_is_ptr && _h.get_contained_ty(lhs->get_type())) {
        /// auto dereference pointers
        from = builder->CreateLoad(from);
      }
      ret = builder->CreateStructGEP(from, (unsigned) p->_access_idx, "member_variable");
      break;
    }
    case MemberAccess::MemberAccessDeref:
      ret = builder->CreateLoad(from);
      break;
    case MemberAccess::MemberAccessMemberFunction:
      ret = codegen(rhs);
      break;
    case MemberAccess::MemberAccessEnumValue:
      ret = nullptr;
      // TODO: codegen of enum_type.enum_value
      break;
    default:
      TAN_ASSERT(false);
  }

  return p->_llvm_value = ret;
}
