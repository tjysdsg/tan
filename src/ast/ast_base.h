#ifndef __TAN_SRC_AST_AST_BASE_H__
#define __TAN_SRC_AST_AST_BASE_H__
#include "base.h"
#include "src/ast/source_traceable.h"
#include "src/ast/precedence.h"
#include "src/ast/ast_node_type.h"
#include "src/ast/fwd.h"
#include <typeinfo>

namespace llvm {
class Value;
}

namespace tanlang {

class ASTBase : public SourceTraceable {
public:
  /// string representation of ASTNodeType
  static umap<ASTNodeType, str> ASTTypeNames;

  /// operator precedence of tokens
  static umap<ASTNodeType, int> OpPrecedence;

public:
  ASTBase() = delete;
  ASTBase(ASTNodeType node_type, int lbp);
  virtual ~ASTBase() = default;

public:
  ASTNodeType get_node_type() const;
  void set_node_type(ASTNodeType node_type);
  void set_lbp(int lbp);
  int get_lbp() const;

  void set_scope(Scope *scope);
  Scope *get_scope() const;

protected:
  virtual str to_string(bool print_prefix = true);

public:
  llvm::Value *_llvm_value = nullptr;

private:
  ASTNodeType _node_type = ASTNodeType::INVALID;
  int _lbp = 0;
  Scope *_scope = nullptr;
};

template<typename To, typename From> To *ast_cast(From *node) {
  static_assert(std::is_base_of<ASTBase, From>::value, "node can only be a subclass of ASTBase");
  return cast_ptr<To, From>(node);
}

template<typename To, typename From> To *ast_must_cast(From *node) {
  auto ret = ast_cast<To, From>(node);
  TAN_ASSERT(ret);
  return ret;
}

}

#endif //__TAN_SRC_AST_AST_BASE_H__
