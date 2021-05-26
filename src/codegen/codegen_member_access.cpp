#include "src/codegen/code_generator_impl.h"
#include "src/llvm_include.h"
#include "compiler_session.h"

using namespace tanlang;

Value *CodeGeneratorImpl::codegen_member_access(ASTMemberAccessPtr p) {
  auto *builder = cs->_builder;
  set_current_debug_location(cs, p);
  auto lhs = p->_children[0];
  ASTNodePtr rhs = nullptr;
  if (p->_children.size() >= 2) { rhs = p->_children[1]; } /// pointer access only have 1 child node
  auto *from = codegen(cs, lhs);
  Value *ret;
  switch (p->_access_type) {
    case MemberAccessType::MemberAccessBracket: {
      if (lhs->_ty->_is_lvalue) { from = builder->CreateLoad(from); }
      auto *rhs_val = codegen(cs, rhs);
      if (rhs->_ty->_is_lvalue) { rhs_val = builder->CreateLoad(rhs_val); }
      ret = builder->CreateGEP(from, rhs_val, "bracket_access");
      break;
    }
    case MemberAccessType::MemberAccessMemberVariable: {
      if (lhs->_ty->_is_lvalue && lhs->_ty->_is_ptr && get_contained_ty(cs, lhs->_ty)) {
        /// auto dereference pointers
        from = builder->CreateLoad(from);
      }
      ret = builder->CreateStructGEP(from, (unsigned) p->_access_idx, "member_variable");
      break;
    }
    case MemberAccessType::MemberAccessDeref:
      ret = builder->CreateLoad(from);
      break;
    case MemberAccessType::MemberAccessMemberFunction:
      ret = codegen(cs, p->_children[1]);
      break;
    case MemberAccessType::MemberAccessEnumValue:
      ret = nullptr;
      // TODO: codegen of enum_type.enum_value
      break;
    default:
      TAN_ASSERT(false);
  }
  p->_llvm_value = ret;
  return ret;
}

