#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include "base.h"
#include "src/ast/precedence.h"
#include "src/ast/ast_type.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/valued_ast_node.h"
#include "src/ast/source_traceable.h"
#include <variant>

namespace llvm {
class Value;
class Type;
class Metadata;
}

namespace tanlang {

/// \section Forward declarations
#define AST_FWD_DECL(c)  \
class c;                 \
using c##Ptr = ptr<c>

AST_FWD_DECL(ASTTy);
AST_FWD_DECL(ASTNode);
struct Scope;
class CompilerSession;
class Parser;
struct Token;
enum class Ty : uint64_t;

class ASTNode : public ParsableASTNode, public ValuedASTNode {
public:
  ASTNode() = delete;
  ASTNode(ASTType op, int lbp);
  virtual ~ASTNode() = default;

  void set_value(str str_value) = 0;
  void set_value(uint64_t int_value) = 0;
  void set_value(double float_value) = 0;
  uint64_t get_int_value();
  str get_str_value();
  double get_float_value();

public:
  ASTTyPtr _ty = nullptr;
  ptr<Scope> _scope = nullptr;
  bool _is_typed = false;
  bool _is_valued = false;
  bool _is_named = false;
  size_t _dominant_idx = 0;

  llvm::Value *_llvm_value = nullptr;
};

template<typename T> std::shared_ptr<T> ast_cast(ptr<SourceTraceable> node) {
  return std::reinterpret_pointer_cast<T>(node);
}

// TODO: replace some ast_cast calls with ast_must_cast if suitable
template<typename T> std::shared_ptr<T> ast_must_cast(ptr<SourceTraceable> node) {
  auto ret = std::reinterpret_pointer_cast<T>(node);
  TAN_ASSERT(ret);
  return ret;
}

#undef AST_FWD_DECL

} // namespace tanlang

#endif /* TAN_SRC_AST_ASTNODE_H_ */
