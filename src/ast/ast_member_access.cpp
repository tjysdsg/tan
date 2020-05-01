#include "src/ast/ast_member_access.h"
#include "src/ast/ast_struct.h"
#include "src/ast/astnode.h"
#include "src/common.h"
#include "compiler_session.h"
#include "compiler.h"

namespace tanlang {

Value *ASTMemberAccess::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  auto lhs = _children[0];
  ASTNodePtr rhs = nullptr;
  if (_children.size() >= 2) { rhs = _children[1]; } // pointer access only have 1 child node
  auto *from = _children[0]->codegen(compiler_session);
  Value *ret = nullptr;
  switch (_access_type) {
    case MemberAccessBracket: {
      if (lhs->is_lvalue()) { from = compiler_session->get_builder()->CreateLoad(from); }
      auto *rhs_val = rhs->codegen(compiler_session);
      if (rhs->is_lvalue()) { rhs_val = compiler_session->get_builder()->CreateLoad(rhs_val); }
      ret = compiler_session->get_builder()->CreateGEP(from, rhs_val, "bracket_access");
      break;
    }
    case MemberAccessMemberVariable: {
      if (lhs->is_lvalue() && lhs->get_ty()->is_ptr() && lhs->get_ty()->get_contained_ty()) {
        from = compiler_session->get_builder()->CreateLoad(from);
      }
      ret = compiler_session->get_builder()->CreateStructGEP(from, (unsigned) _access_idx, "member_variable");
      break;
    }
    case MemberAccessDeref:
      ret = compiler_session->get_builder()->CreateLoad(from);
      break;
    case MemberAccessMemberFunction:
      // TODO: codegen for member function call
      break;
    default:
      TAN_ASSERT(false);
      break;
  }
  _llvm_type = ret->getType();
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

  ASTNodePtr lhs = left;
  if (_access_type == MemberAccessBracket) {
    ++_end_index; // skip "]" if this is a bracket access
    _ty = left->get_ty();
    TAN_ASSERT(_ty->is_ptr());
    _ty = _ty->get_contained_ty();
    _type_name = _ty->get_type_name();
    if (!_ty) { report_code_error(_token, "Unable to perform bracket access"); }
  } else if (_access_type == MemberAccessDeref) { /// pointer dereference
    _ty = left->get_ty();
    TAN_ASSERT(_ty->is_ptr());
    _ty = _ty->get_contained_ty();
    _type_name = _ty->get_type_name();
  } else if (_children[1]->_type == ASTType::ID) { /// member variable
    _access_type = MemberAccessMemberVariable;
    lhs = _children[0];
    if (!lhs->is_lvalue() && !lhs->get_ty()->is_ptr()) { report_code_error(_token, "Invalid left-hand operand"); }
    auto rhs = _children[1];
    std::string m_name = rhs->get_name();
    auto struct_ast = ast_cast<ASTStruct>(_cs->get(lhs->get_type_name()));
    _access_idx = struct_ast->get_member_index(m_name);
    auto member = struct_ast->get_member(_access_idx);
    _type_name = member->get_type_name();
    _ty = member->get_ty();
  } else if (_children[1]->_type == ASTType::FUNC_CALL) {
    // TODO: member function call
    _access_type = MemberAccessMemberFunction;
    TAN_ASSERT(false);
  } else { report_code_error(_token, "Invalid right-hand operand"); }
  return _end_index;
}

llvm::Type *ASTMemberAccess::to_llvm_type(CompilerSession *) const { return _llvm_type; }

std::string ASTMemberAccess::get_type_name() const { return _type_name; }

llvm::Value *ASTMemberAccess::get_llvm_value(CompilerSession *) const { return _llvm_value; }

std::shared_ptr<ASTTy> ASTMemberAccess::get_ty() const {
  TAN_ASSERT(_ty);
  return _ty;
}

ASTMemberAccess::ASTMemberAccess(Token *token, size_t token_index) : ASTNode(ASTType::MEMBER_ACCESS,
    op_precedence[ASTType::MEMBER_ACCESS],
    0,
    token,
    token_index) {}

bool ASTMemberAccess::is_lvalue() const { return true; }

bool ASTMemberAccess::is_typed() const { return true; }

} // namespace tanlang
