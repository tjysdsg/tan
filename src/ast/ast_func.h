#ifndef TAN_SRC_AST_AST_FUNC_H_
#define TAN_SRC_AST_AST_FUNC_H_
#include "src/ast/astnode.h"
#include "src/llvm_include.h"

namespace tanlang {
struct Token;

class ASTFunction: public ASTNode {
 public:
  explicit ASTFunction(Token* token);
  Value *codegen(ParserContext *parser_context) override;
  void nud(Parser *parser) override;
};
}

#endif /* TAN_SRC_AST_AST_FUNC_H_ */
