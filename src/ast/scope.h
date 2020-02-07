#ifndef TAN_SRC_AST_SCOPE_H_
#define TAN_SRC_AST_SCOPE_H_
#include <unordered_map>
#include <string>
#include "src/ast/astnode.h"

namespace tanlang {

struct Scope {
  std::unordered_map<std::string, Value *> _named;
  BasicBlock* _code_block = nullptr;
};

}

#endif //TAN_SRC_AST_SCOPE_H_
