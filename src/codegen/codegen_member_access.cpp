#include "src/codegen/code_generator_impl.h"
#include "src/llvm_include.h"
#include "src/ast/ast_member_access.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"

using namespace tanlang;

Value *CodeGeneratorImpl::codegen_member_access(const ASTMemberAccessPtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);
  auto lhs = p->get_child_at<ASTNode>(0);
  ASTNodePtr rhs = nullptr;
  if (p->get_children_size() >= 2) {
    rhs = p->get_child_at<ASTNode>(1);
  } /// pointer access only have 1 child node
  auto *from = codegen(lhs);
  Value *ret;
  switch (p->_access_type) {
    case MemberAccessType::MemberAccessBracket: {
      if (lhs->_ty->_is_lvalue) { from = builder->CreateLoad(from); }
      auto *rhs_val = codegen(rhs);
      if (rhs->_ty->_is_lvalue) { rhs_val = builder->CreateLoad(rhs_val); }
      ret = builder->CreateGEP(from, rhs_val, "bracket_access");
      break;
    }
    case MemberAccessType::MemberAccessMemberVariable: {
      if (lhs->_ty->_is_lvalue && lhs->_ty->_is_ptr && _h.get_contained_ty(lhs->_ty)) {
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
      ret = codegen(p->get_child_at<ASTNode>(1));
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

