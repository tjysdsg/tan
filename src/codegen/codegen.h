#ifndef __TAN_SRC_CODEGEN_CODEGEN_H__
#define __TAN_SRC_CODEGEN_CODEGEN_H__
#include "src/ast/ast_node.h"

namespace llvm {
class Value;
class Type;
}

namespace tanlang {

llvm::Value *codegen(CompilerSession *cs, ASTNodePtr p);

} // namespace tanlang

#endif //__TAN_SRC_CODEGEN_CODEGEN_H__
