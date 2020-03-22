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
  // TODO: implement codegen for struct declaration
  std::vector<Type *> members;
  for (size_t i = 1; i < _children.size(); ++i) {
    auto var = std::reinterpret_pointer_cast<ASTVarDecl>(_children[i]);
    auto m = std::reinterpret_pointer_cast<ASTTy>(var->_children[1]);
    members.push_back(m->to_llvm_type(compiler_session));
  }

  StructType *struct_type = StructType::create(*compiler_session->get_context(), "S");
  struct_type->setBody(members);
  _llvm_type = struct_type;

  auto ty_name = std::reinterpret_pointer_cast<ASTIdentifier>(_children[0]);
  compiler_session->add(ty_name->_name, this->shared_from_this());
  return nullptr;
}

} // namespace tanlang
