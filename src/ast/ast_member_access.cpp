#include "src/common.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

void ASTMemberAccess::resolve_ptr_deref(ASTNodePtr left) {
  TAN_ASSERT(_access_type == MemberAccessDeref);
  _ty = left->get_ty();
  TAN_ASSERT(_ty->is_ptr());
  _ty = std::make_shared<ASTTy>(*_ty->get_contained_ty());
  _ty->set_is_lvalue(true);
}

Value *ASTMemberAccess::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(_token->l, _token->c);
  auto lhs = _children[0];
  ASTNodePtr rhs = nullptr;
  if (_children.size() >= 2) { rhs = _children[1]; } /// pointer access only have 1 child node
  auto *from = _children[0]->codegen(cs);
  Value *ret;
  switch (_access_type) {
    case MemberAccessBracket: {
      if (lhs->is_lvalue()) { from = builder->CreateLoad(from); }
      auto *rhs_val = rhs->codegen(cs);
      if (rhs->is_lvalue()) { rhs_val = builder->CreateLoad(rhs_val); }
      ret = builder->CreateGEP(from, rhs_val, "bracket_access");
      break;
    }
    case MemberAccessMemberVariable: {
      if (lhs->is_lvalue() && lhs->get_ty()->is_ptr() && lhs->get_ty()->get_contained_ty()) {
        /// auto dereference pointers
        from = builder->CreateLoad(from);
      }
      ret = builder->CreateStructGEP(from, (unsigned) _access_idx, "member_variable");
      break;
    }
    case MemberAccessDeref:
      ret = builder->CreateLoad(from);
      break;
    case MemberAccessMemberFunction:
      ret = _children[1]->codegen(cs);
      break;
    case MemberAccessEnumValue:
      ret = ConstantInt::get(_ty->to_llvm_type(cs), _enum_value, !_ty->is_unsigned());
      break;
    default:
      TAN_ASSERT(false);
  }
  _llvm_value = ret;
  TAN_ASSERT(ret);
  return ret;
}

ASTMemberAccessPtr ASTMemberAccess::CreatePointerDeref(ASTNodePtr ptr) {
  auto ret = std::make_shared<ASTMemberAccess>(nullptr, 0);
  ret->_children.push_back(ptr);
  ret->_access_idx = MemberAccessType::MemberAccessDeref;
  ret->resolve_ptr_deref(ptr);
  return ret;
}

} // namespace tanlang
