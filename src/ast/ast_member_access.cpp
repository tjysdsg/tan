#include "src/ast/ast_member_access.h"
#include "src/ast/ast_array.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_struct.h"
#include "src/ast/ast_expr.h"
#include "src/ast/astnode.h"
#include "src/ast/common.h"
#include "compiler_session.h"

namespace tanlang {

Value *ASTMemberAccess::codegen(CompilerSession *compiler_session) {
  Value *ret = nullptr;
  if (_children[1]->_type == ASTType::ID) { /// dot access
    std::shared_ptr<ASTNode> lhs = nullptr;
    unsigned member_index = 0;
    Value *from = nullptr;
    if (_children[0]->_type == ASTType::ID) { /// struct instance access
      if (_children[1]->_type == ASTType::FUNC_CALL) { /// calling a member function
        // TODO: codegen for member function calls
        throw std::runtime_error("NOT IMPLEMENTED");
      }
      /// otherwise member variable
      std::string member_name = ast_cast<ASTIdentifier>(_children[1])->get_name();
      lhs = compiler_session->get(ast_cast<ASTIdentifier>(_children[0])->get_name());
      auto instance = ast_cast<ASTVarDecl>(lhs);
      // TODO: check if member is in the struct
      std::shared_ptr<ASTStruct> struct_ast = ast_cast<ASTStruct>(compiler_session->get(instance->get_type_name()));
      member_index = static_cast<unsigned>(struct_ast->get_member_index(member_name));
      from = instance->get_llvm_value(compiler_session);
    } else if (_children[0]->_type == ASTType::STRUCT_DECL) { /// struct static access
      // TODO: implement dot access for static access
      std::shared_ptr<ASTStruct> struct_ast = ast_cast<ASTStruct>(lhs);
      throw std::runtime_error("NOT IMPLEMENTED");
    } else { /// otherwise access rvalue
      // TODO: implement dot access for rvalue
      from = _children[0]->codegen(compiler_session);
      throw std::runtime_error("NOT IMPLEMENTED");
    }
    ret = compiler_session->get_builder()->CreateStructGEP(from, member_index, "member_ptr");
  } else if (_is_bracket) { /// bracket access
    Value *from = nullptr;
    from = _children[0]->codegen(compiler_session);
    if (_children[0]->is_lvalue()) {
      from = compiler_session->get_builder()->CreateLoad(from);
    }
    auto rhs = _children[1];
    ret = compiler_session->get_builder()->CreateGEP(from, rhs->codegen(compiler_session), "member_ptr");
  } else {
    report_code_error(_token, "Invalid member access");
  }
  return ret;
}

}
