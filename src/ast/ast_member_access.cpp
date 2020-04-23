#include "src/ast/ast_member_access.h"
#include "src/ast/ast_struct.h"
#include "src/ast/astnode.h"
#include "src/ast/common.h"
#include "compiler_session.h"

namespace tanlang {

Value *ASTMemberAccess::codegen_dot_member_variable(CompilerSession *compiler_session, ASTNodePtr lhs, ASTNodePtr rhs) {
  assert(lhs->is_typed());
  assert(lhs->is_lvalue());
  assert(rhs->_type == ASTType::ID);
  std::string member_name = rhs->get_name();
  auto struct_ast = ast_cast<ASTStruct>(compiler_session->get(lhs->get_type_name()));
  unsigned member_index = static_cast<unsigned>(struct_ast->get_member_index(member_name));
  Value *from = lhs->get_llvm_value(compiler_session);
  { /// set type name
    auto member = struct_ast->get_member(member_index);
    _type_name = member->get_type_name();
  }
  return compiler_session->get_builder()->CreateStructGEP(from, member_index, "member_ptr");
}

Value *ASTMemberAccess::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  auto lhs = _children[0];
  auto rhs = _children[1];
  auto *lhs_val = _children[0]->codegen(compiler_session);
  Value *ret = nullptr;
  if (_is_bracket) { /// bracket access
    if (lhs->is_lvalue()) {
      lhs_val = compiler_session->get_builder()->CreateLoad(lhs_val);
    }
    auto *rhs_val = rhs->codegen(compiler_session);
    ret = compiler_session->get_builder()->CreateGEP(lhs_val, rhs_val, "member_ptr");
    // TODO: set _type_name
  } else if (rhs->_type == ASTType::ID) { /// dot access
    if (is_ast_type_in(lhs->_type, {ASTType::MEMBER_ACCESS, ASTType::ID})) { /// struct instance access
      if (lhs->is_named()) { lhs = compiler_session->get(lhs->get_name()); }
      ret = codegen_dot_member_variable(compiler_session, lhs, rhs);
    } else { /// struct static access
      // TODO: implement dot access for static access
      throw std::runtime_error("NOT IMPLEMENTED");
    }
  } else if (rhs->_type == ASTType::FUNC_CALL) { /// calling a member function
    // TODO: codegen for member function calls
    throw std::runtime_error("NOT IMPLEMENTED");
  } else {
    report_code_error(_token, "Invalid member access");
  }
  _llvm_type = ret->getType();
  _llvm_value = ret;
  assert(ret);
  return ret;
}

llvm::Type *ASTMemberAccess::to_llvm_type(CompilerSession *) const { return _llvm_type; }

std::string ASTMemberAccess::get_type_name() const { return _type_name; }

llvm::Value *ASTMemberAccess::get_llvm_value(CompilerSession *) const {
  return _llvm_value;
}

} // namespace tanlang
