#ifndef TAN_SRC_AST_SCOPE_H_
#define TAN_SRC_AST_SCOPE_H_
#include <unordered_map>
#include <string>
#include <memory>

namespace tanlang {

class ASTNode;
using ASTNodePtr = std::shared_ptr<ASTNode>;

struct Scope {
  std::unordered_map<std::string, ASTNodePtr> _named{}; /// named identifiers in this scope
};

} // namespace tanlang

#endif //TAN_SRC_AST_SCOPE_H_
