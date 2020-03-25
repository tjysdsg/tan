#include "ast_struct.h"
#include "compiler_session.h"
#include "src/llvm_include.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_expr.h"

namespace tanlang {

ASTStruct::ASTStruct(Token *token) : ASTNode(ASTType::STRUCT_DECL, 0, 0, token) {}

Value *ASTStruct::codegen(CompilerSession *compiler_session) {
  using llvm::StructType;
  auto ty_name = ast_cast<ASTIdentifier>(_children[0])->get_name(); // name of this struct
  std::vector<Type *> members;
  for (size_t i = 1; i < _children.size(); ++i) {
    auto var = ast_cast<ASTVarDecl>(_children[i]);
    members.push_back(var->to_llvm_type(compiler_session));
    _member_indices[var->get_name()] = i - 1;
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

std::string ASTStruct::get_name() const {
  return ast_cast<ASTIdentifier>(_children[0])->get_name();
}

std::string ASTStruct::get_type_name() const {
  return ast_cast<ASTTy>(_children[1])->get_type_name();
}

llvm::Type *ASTStruct::to_llvm_type(CompilerSession *) const {
  return _llvm_type;
}

} // namespace tanlang
