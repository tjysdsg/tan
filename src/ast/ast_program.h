#ifndef __TAN_SRC_AST_AST_STATEMENT_CPP_AST_PROGRAM_H__
#define __TAN_SRC_AST_AST_STATEMENT_CPP_AST_PROGRAM_H__
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTProgram final : public tanlang::ASTNode {
public:
  ASTProgram();
  llvm::Value *codegen(CompilerSession *) override;

protected:
  size_t nud() override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_STATEMENT_CPP_AST_PROGRAM_H__ */
