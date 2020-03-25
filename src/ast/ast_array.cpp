#include "ast_array.h"

namespace tanlang {

Value *ASTArrayLiteral::codegen(CompilerSession *compiler_session) {
  return ASTNode::codegen(compiler_session);
}

} // namespace tanlang
