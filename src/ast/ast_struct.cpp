#include "ast_struct.h"
#include "compiler_session.h"

namespace tanlang {

ASTStruct::ASTStruct(Token *token) : ASTNode(ASTType::STRUCT_DECL, 0, 0, token) {}

Value *ASTStruct::codegen(CompilerSession *compiler_session) {
  // TODO: implement codegen for struct declaration
  return nullptr;
}

} // namespace tanlang
