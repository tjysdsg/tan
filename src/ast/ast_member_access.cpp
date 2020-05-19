#include "src/ast/ast_member_access.h"
#include "src/ast/ast_enum.h"
#include "src/ast/ast_number_literal.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_ampersand.h"
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

size_t ASTMemberAccess::led(const ASTNodePtr &left) {
  _end_index = _start_index + 1; /// skip "." or "["
  _children.push_back(left); /// lhs
  if (_parser->at(_start_index)->value == "[") { _access_type = MemberAccessBracket; }
  auto right = _parser->peek(_end_index);
  if (_access_type != MemberAccessBracket && right->_token->value == "*") { /// pointer dereference
    _access_type = MemberAccessDeref;
    ++_end_index;
  } else if (right->_type != ASTType::FUNC_CALL) {
    /// postpone parsing of function call, because we want to insert `self` as the first argument
    _end_index = right->parse(_parser, _cs);
    _children.push_back(right);
  } else {
    _children.push_back(right);
  }

  if (_access_type == MemberAccessBracket) {
    auto rhs = _children[1];
    ++_end_index; /// skip "]" if this is a bracket access
    _ty = left->get_ty();
    TAN_ASSERT(_ty->is_ptr());
    _ty = std::make_shared<ASTTy>(*_ty->get_contained_ty());
    _ty->set_is_lvalue(true);
    if (!_ty) { error("Unable to perform bracket access"); }
    // TODO: check bound if rhs is compile-time known
    if (rhs->_type == ASTType::NUM_LITERAL) {
      if (!rhs->get_ty()->is_int()) { error("Expect an integer specifying array size"); }
      auto size = ast_cast<ASTNumberLiteral>(rhs);
      if (left->get_ty()->is_array()
          && (size_t) /** underflow helps us here */ size->_ivalue >= left->get_ty()->get_n_elements()) {
        error("Index " + std::to_string(size->_ivalue) + " out of bound, the array size is "
            + std::to_string(left->get_ty()->get_n_elements()));
      }
    }
  } else if (_access_type == MemberAccessDeref) { /// pointer dereference
    resolve_ptr_deref(left);
  } else if (_children[1]->_type == ASTType::ID) { /// member variable or enum
    if (left->get_ty()->is_enum()) {
      _access_type = MemberAccessEnumValue;
      auto enum_ = ast_cast<ASTEnum>(left->get_ty());
      _ty = enum_;
      _enum_value = enum_->get_enum_value(_children[1]->get_name());
    } else {
      _access_type = MemberAccessMemberVariable;
      if (!left->is_lvalue() && !left->get_ty()->is_ptr()) { error("Invalid left-hand operand"); }
      auto rhs = _children[1];
      str m_name = rhs->get_name();
      std::shared_ptr<ASTTy> struct_ast = nullptr;
      /// auto dereference pointers
      if (left->get_ty()->is_ptr()) {
        struct_ast = ast_cast<ASTTy>(_cs->get(left->get_ty()->get_contained_ty()->get_type_name()));
      } else {
        struct_ast = ast_cast<ASTTy>(_cs->get(left->get_type_name()));
      }
      _access_idx = struct_ast->get_member_index(m_name);
      auto member = struct_ast->get_member(_access_idx);
      _ty = std::make_shared<ASTTy>(*member->get_ty());
      _ty->set_is_lvalue(true);
    }
  } else if (_children[1]->_type == ASTType::FUNC_CALL) { /// method call
    /// TODO: make `self` reference instead of pointer
    auto func = ast_cast<ASTFunctionCall>(_children[1]);
    TAN_ASSERT(func);
    func->_do_resolve = false;
    _end_index = func->parse(_parser, _cs);
    if (!left->is_lvalue() && !left->get_ty()->is_ptr()) {
      error("Method calls require left-hand operand to be an lvalue or a pointer");
    }
    /// auto dereference pointers
    if (left->is_lvalue() && !left->get_ty()->is_ptr()) {
      func->_children.insert(func->_children.begin(), ASTAmpersand::CreateAddressOf(left));
    } else {
      func->_children.insert(func->_children.begin(), left);
    }
    func->resolve();
    _ty = func->get_ty();
    _access_type = MemberAccessMemberFunction;
  } else { error("Invalid right-hand operand"); }
  return _end_index;
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

ASTMemberAccess::ASTMemberAccess(Token *t, size_t ti) : ASTNode(ASTType::MEMBER_ACCESS,
    op_precedence[ASTType::MEMBER_ACCESS],
    0,
    t,
    ti) {}

bool ASTMemberAccess::is_lvalue() { return true; }

bool ASTMemberAccess::is_typed() { return true; }

} // namespace tanlang
