#include "src/ast/ast_member_access.h"
#include "src/ast/ast_number_literal.h"
#include "src/ast/ast_struct.h"
#include "src/common.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

Value *ASTMemberAccess::codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  auto lhs = _children[0];
  ASTNodePtr rhs = nullptr;
  if (_children.size() >= 2) { rhs = _children[1]; } // pointer access only have 1 child node
  auto *from = _children[0]->codegen(cs);
  Value *ret = nullptr;
  switch (_access_type) {
    case MemberAccessBracket: {
      if (lhs->is_lvalue()) { from = cs->get_builder()->CreateLoad(from); }
      auto *rhs_val = rhs->codegen(cs);
      if (rhs->is_lvalue()) { rhs_val = cs->get_builder()->CreateLoad(rhs_val); }
      ret = cs->get_builder()->CreateGEP(from, rhs_val, "bracket_access");
      break;
    }
    case MemberAccessMemberVariable: {
      if (lhs->is_lvalue() && lhs->get_ty()->is_ptr() && lhs->get_ty()->get_contained_ty()) {
        /// auto dereference pointers
        from = cs->get_builder()->CreateLoad(from);
      }
      ret = cs->get_builder()->CreateStructGEP(from, (unsigned) _access_idx, "member_variable");
      break;
    }
    case MemberAccessDeref:
      ret = cs->get_builder()->CreateLoad(from);
      break;
    case MemberAccessMemberFunction:
      // TODO: codegen for member function call
      break;
    default:
      TAN_ASSERT(false);
      break;
  }
  _llvm_value = ret;
  TAN_ASSERT(ret);
  return ret;
}

size_t ASTMemberAccess::led(const ASTNodePtr &left) {
  _end_index = _start_index + 1; /// skip "." or "["
  _children.push_back(left); /// lhs
  TAN_ASSERT(left->is_typed());
  if (_parser->at(_start_index)->value == "[") { _access_type = MemberAccessBracket; }
  auto member_name = _parser->peek(_end_index);
  if (_access_type != MemberAccessBracket && member_name->_token->value == "*") { /// pointer dereference
    _access_type = MemberAccessDeref;
    ++_end_index;
  } else {
    _end_index = member_name->parse(_parser, _cs);
    _children.push_back(member_name);
  }

  if (_access_type == MemberAccessBracket) {
    auto rhs = _children[1];
    ++_end_index; /// skip "]" if this is a bracket access
    _ty = left->get_ty();
    TAN_ASSERT(_ty->is_ptr());
    _ty = _ty->get_contained_ty();
    if (!_ty) { report_code_error(_token, "Unable to perform bracket access"); }
    // TODO: check bound if rhs is compile-time known
    if (rhs->_type == ASTType::NUM_LITERAL) {
      if (!rhs->get_ty()->is_int()) { report_code_error(_token, "Expect an integer specifying array size"); }
      auto size = ast_cast<ASTNumberLiteral>(rhs);
      if (left->get_ty()->is_array()
          && (size_t) /** underflow helps us here */ size->_ivalue >= left->get_ty()->get_n_elements()) {
        report_code_error(_token,
            "Index " + std::to_string(size->_ivalue) + " out of bound, the array size is "
                + std::to_string(left->get_ty()->get_n_elements()));
      }
    }
  } else if (_access_type == MemberAccessDeref) { /// pointer dereference
    _ty = left->get_ty();
    TAN_ASSERT(_ty->is_ptr());
    _ty = _ty->get_contained_ty();
  } else if (_children[1]->_type == ASTType::ID) { /// member variable
    _access_type = MemberAccessMemberVariable;
    if (!left->is_lvalue() && !left->get_ty()->is_ptr()) { report_code_error(_token, "Invalid left-hand operand"); }
    auto rhs = _children[1];
    str m_name = rhs->get_name();
    std::shared_ptr<ASTStruct> struct_ast = nullptr;
    /// auto dereference pointers
    if (left->get_ty()->is_ptr()) {
      struct_ast = ast_cast<ASTStruct>(_cs->get(left->get_ty()->get_contained_ty()->get_type_name()));
    } else { struct_ast = ast_cast<ASTStruct>(_cs->get(left->get_type_name())); }
    _access_idx = struct_ast->get_member_index(m_name);
    auto member = struct_ast->get_member(_access_idx);
    _ty = member->get_ty();
  } else if (_children[1]->_type == ASTType::FUNC_CALL) {
    // TODO: member function call
    _access_type = MemberAccessMemberFunction;
    TAN_ASSERT(false);
  } else { report_code_error(_token, "Invalid right-hand operand"); }
  return _end_index;
}

ASTMemberAccess::ASTMemberAccess(Token *token, size_t token_index) : ASTNode(ASTType::MEMBER_ACCESS,
    op_precedence[ASTType::MEMBER_ACCESS],
    0,
    token,
    token_index) {}

bool ASTMemberAccess::is_lvalue() const { return true; }

bool ASTMemberAccess::is_typed() const { return true; }

} // namespace tanlang
