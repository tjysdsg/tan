#ifndef TAN_SRC_AST_SCOPE_H_
#define TAN_SRC_AST_SCOPE_H_
#include <unordered_map>
#include <string>
#include "src/ast/astnode.h"

namespace tanlang {

struct StackTrace;

struct Scope {
  // TODO: separate types with named
  std::unordered_map<std::string, std::shared_ptr<ASTNode>> _named{}; ///< named identifiers in this scope
  BasicBlock *_code_block = nullptr; ///< parent code block
};

} // namespace tanlang

#endif //TAN_SRC_AST_SCOPE_H_
