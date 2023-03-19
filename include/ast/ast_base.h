#ifndef __TAN_SRC_AST_AST_BASE_H__
#define __TAN_SRC_AST_AST_BASE_H__
#include "base.h"
#include "source_traceable.h"
#include "precedence.h"
#include "ast_node_type.h"
#include <typeinfo>

namespace tanlang {

class Context;

class ASTBase : public SourceTraceable {
public:
  /// string representation of ASTNodeType
  static umap<ASTNodeType, str> ASTTypeNames;

  /// operator precedence of tokens
  static umap<ASTNodeType, int> OpPrecedence;

public:
  ASTBase() = delete;
  ASTBase(ASTNodeType node_type, SrcLoc loc, int bp);
  virtual ~ASTBase() = default;

public:
  [[nodiscard]] ASTNodeType get_node_type() const;
  void set_node_type(ASTNodeType node_type);
  [[nodiscard]] int get_bp() const;
  [[nodiscard]] Context *ctx();

  /**
   * \brief Get a ordered list of child nodes
   */
  [[nodiscard]] virtual vector<ASTBase *> get_children() const;

  /// Pretty-print AST tree
  void printTree() const;

protected:
  [[nodiscard]] virtual str to_string(bool print_prefix = true) const;

  /**
   * \brief Get the "actual" this. Used for implementing proxy classes.
   */
  [[nodiscard]] virtual ASTBase *get() const;

private:
  void printTree(const str &prefix, bool last_child) const;

private:
  ASTNodeType _node_type;
  int _bp = 0; /// binding power
  Context *_ctx = nullptr;
};

/**
 * \brief A helper function to cast between pointers to different AST node types, with safety checks in DEBUG mode.
 * \details Uses dynamic_cast for runtime type checking in DEBUG mode, C-style type casting in RELEASE mode.
 * \tparam To Target type
 * \tparam From Optional, source type
 * \param p Pointer
 * \return Converted pointer
 */
template <typename To, typename From> To *ast_cast(From *p) {
  static_assert(std::is_base_of<ASTBase, From>::value, "Input type can only be a subclass of ASTBase");
  static_assert(std::is_base_of<ASTBase, To>::value, "Target type can only be a subclass of ASTBase");

#ifdef DEBUG
  auto *ret = dynamic_cast<To *>(p);
#else
  auto *ret = (To *)p;
#endif

  TAN_ASSERT(ret);
  return ret;
}

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_BASE_H__
