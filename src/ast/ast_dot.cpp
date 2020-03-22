#include "src/ast/ast_dot.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_struct.h"
#include "src/ast/ast_expr.h"
#include "src/ast/astnode.h"
#include "compiler_session.h"

namespace tanlang {

Value *ASTDot::codegen(CompilerSession *compiler_session) {
  if (_children[1]->_type == ASTType::ID) {
    std::string member_name = std::reinterpret_pointer_cast<ASTIdentifier>(_children[1])->_name;
    std::shared_ptr<ASTStruct> struct_ast = nullptr;
    auto tmp = compiler_session->get(ast_cast<ASTIdentifier>(_children[0])->_name);
    Value *ret = nullptr;
    if (tmp->_type == ASTType::STRUCT_DECL) { // static access
      // TODO: static access
      struct_ast = ast_cast<ASTStruct>(tmp);
      throw std::runtime_error("NOT IMPLEMENTED");
    } else { // instance access
      auto instance = ast_cast<ASTVarDecl>(tmp);
      struct_ast = ast_cast<ASTStruct>(compiler_session->get(instance->_ty->_type_name));
      std::string struct_name = ast_cast<ASTIdentifier>(struct_ast->_children[0])->_name;
      unsigned member_index = static_cast<unsigned>(struct_ast->get_member_index(member_name));
      llvm::Value *member_ptr =
          compiler_session->get_builder()->CreateStructGEP(instance->_llvm_value, member_index, "member_ptr");
      ret = member_ptr;
    }
    return ret;
  } else if (_children[1]->_type == ASTType::FUNC_CALL) {
    // TODO: codegen for member function calls
    return nullptr;
  }
  return nullptr;
}

}
