#include "ast_struct.h"
#include "compiler_session.h"
#include "src/ast/ast_ty.h"
#include "src/llvm_include.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_expr.h"

namespace tanlang {

ASTStruct::ASTStruct(Token *token) : ASTNode(ASTType::STRUCT_DECL, 0, 0, token) {}

Value *ASTStruct::codegen(CompilerSession *compiler_session) {
  using llvm::StructType;
  auto ty_name = std::reinterpret_pointer_cast<ASTIdentifier>(_children[0])->_name;
  // TODO: implement codegen for struct declaration
  std::vector<Type *> members;
  for (size_t i = 1; i < _children.size(); ++i) {
    auto var = std::reinterpret_pointer_cast<ASTVarDecl>(_children[i]);
    // TODO: use a function like ASTVarDecl.get_name() instead of direct access
    auto m = std::reinterpret_pointer_cast<ASTTy>(var->_children[1]);
    members.push_back(m->to_llvm_type(compiler_session));
    std::string member_name = std::reinterpret_pointer_cast<ASTIdentifier>(var->_children[0])->_name;
    _member_indices[member_name] = i - 1;
  }

  StructType *struct_type = StructType::create(*compiler_session->get_context(), ty_name);
  struct_type->setBody(members);
  _llvm_type = struct_type;

  compiler_session->add(ty_name, this->shared_from_this());
  return nullptr;
}

size_t ASTStruct::get_member_index(std::string name) {
  return _member_indices[name];
}

} // namespace tanlang
