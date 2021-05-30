#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include "base.h"
#include "src/ast/precedence.h"
#include "src/ast/ast_node_type.h"
#include "src/ast/ast_base.h"
#include "src/ast/source_traceable.h"
#include <variant>

namespace llvm {
class Value;
class Type;
class Metadata;
}

namespace tanlang {

AST_FWD_DECL(ASTType);
AST_FWD_DECL(ASTNode);
class CompilerSession;
class Parser;
struct Token;
enum class Ty : uint64_t;

class ASTNode : public ASTBase {
public:
  ASTNode() = delete;
  ASTNode(ASTNodeType op, int lbp);
  virtual ~ASTNode() = default;

public:
  bool _is_typed = false;
  bool _is_valued = false;
  bool _is_named = false;
  size_t _dominant_idx = 0;

  llvm::Value *_llvm_value = nullptr;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_ASTNODE_H_ */
