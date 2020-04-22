#include "src/ast/ast_member_access.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_struct.h"
#include "src/ast/astnode.h"
#include "src/ast/common.h"
#include "compiler_session.h"

namespace tanlang {

Value *ASTMemberAccess::codegen_dot_member_variable(CompilerSession *compiler_session) {
  std::string member_name = ast_cast<ASTIdentifier>(_children[1])->get_name();
  auto lhs = compiler_session->get(ast_cast<ASTIdentifier>(_children[0])->get_name());
  assert(lhs->is_typed());
  auto struct_ast = ast_cast<ASTStruct>(compiler_session->get(lhs->get_type_name()));
  unsigned member_index = static_cast<unsigned>(struct_ast->get_member_index(member_name));
  Value *from = lhs->get_llvm_value(compiler_session);
  return compiler_session->get_builder()->CreateStructGEP(from, member_index, "member_ptr");
}

Value *ASTMemberAccess::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  Value *ret = nullptr;
  if (_is_bracket) { /// bracket access
    auto *from = _children[0]->codegen(compiler_session);
    if (_children[0]->is_lvalue()) {
      from = compiler_session->get_builder()->CreateLoad(from);
    }
    auto rhs = _children[1];
    ret = compiler_session->get_builder()->CreateGEP(from, rhs->codegen(compiler_session), "member_ptr");
  } else if (_children[1]->_type == ASTType::ID) { /// dot access
    if (_children[0]->_type == ASTType::ID) { /// struct instance access
      /// otherwise member variable
      ret = codegen_dot_member_variable(compiler_session);
    } else if (_children[0]->_type == ASTType::STRUCT_DECL) { /// struct static access
      // TODO: implement dot access for static access
      throw std::runtime_error("NOT IMPLEMENTED");
    } else { /// otherwise access rvalue
      // TODO: implement dot access for rvalue
    }
  } else if (_children[1]->_type == ASTType::FUNC_CALL) { /// calling a member function
    // TODO: codegen for member function calls
    throw std::runtime_error("NOT IMPLEMENTED");
  } else {
    report_code_error(_token, "Invalid member access");
  }
  _llvm_type = ret->getType();
  assert(ret);
  return ret;
}

llvm::Type *ASTMemberAccess::to_llvm_type(CompilerSession *) const { return _llvm_type; }

std::string ASTMemberAccess::get_type_name() const { return _type_name; }

} // namespace tanlang
