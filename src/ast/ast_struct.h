#ifndef __TAN_SRC_AST_AST_STRUCT_H__
#define __TAN_SRC_AST_AST_STRUCT_H__
#include "src/ast/astnode.h"

namespace tanlang {
class ASTStruct : public ASTNode, public std::enable_shared_from_this<ASTStruct> {
public:
  ASTStruct() = delete;
  explicit ASTStruct(Token *token);
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;

public:
  llvm::Type *_llvm_type = nullptr;
};
} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_STRUCT_H__ */
