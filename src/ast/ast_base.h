#ifndef __TAN_SRC_AST_AST_BASE_H__
#define __TAN_SRC_AST_AST_BASE_H__
#include "base.h"
#include "src/ast/source_traceable.h"
#include "src/ast/precedence.h"
#include "src/ast/ast_node_type.h"
#include <variant>
#ifdef DEBUG
#include <typeinfo>
#endif

namespace llvm {
class Value;
}

namespace tanlang {

AST_FWD_DECL(ASTBase);
struct Scope;

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

  void set_scope(const ptr<Scope> &scope);
  ptr<Scope> get_scope() const;

protected:
  virtual str to_string(bool print_prefix = true);

public:
  llvm::Value *_llvm_value = nullptr;

private:
  ASTNodeType _node_type = ASTNodeType::INVALID;
  int _lbp = 0;
  ptr<Scope> _scope = nullptr;
};

template<typename T, typename C> std::shared_ptr<T> ast_cast(ptr<C> node) {
  static_assert(std::is_base_of<ASTBase, C>::value, "node can only be a subclass of ASTBase");
  #ifdef DEBUG
  auto ret = std::dynamic_pointer_cast<T>(node);
  #else
  auto ret = std::reinterpret_pointer_cast<T>(node);
  #endif
  return ret;
}

template<typename T, typename C> std::shared_ptr<T> ast_must_cast(ptr<C> node) {
  static_assert(std::is_base_of<ASTBase, C>::value, "node can only be a subclass of ASTBase");
  #ifdef DEBUG
  auto ret = std::dynamic_pointer_cast<T>(node);
  #else
  auto ret = std::reinterpret_pointer_cast<T>(node);
  #endif
  TAN_ASSERT(ret);
  return ret;
}

}

#endif //__TAN_SRC_AST_AST_BASE_H__
