#ifndef TAN_SRC_AST_SCOPE_H_
#define TAN_SRC_AST_SCOPE_H_
#include <unordered_map>
#include <string>
#include <memory>
#include "src/llvm_include.h"

namespace tanlang {

class ASTNode;

using ASTNodePtr = std::shared_ptr<ASTNode>;

struct StackTrace;

struct Scope {
  std::unordered_map<std::string, ASTNodePtr> _named{}; ///< named identifiers in this scope
  BasicBlock *_code_block = nullptr; ///< parent code block
};

} // namespace tanlang

#endif //TAN_SRC_AST_SCOPE_H_
