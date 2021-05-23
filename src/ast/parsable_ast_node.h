#ifndef __TAN_SRC_AST_PARSABLE_AST_NODE_H__
#define __TAN_SRC_AST_PARSABLE_AST_NODE_H__
#include "base.h"
#include "src/ast/source_traceable.h"
#include "src/ast/precedence.h"
#include "src/ast/ast_type.h"
#include <variant>

namespace tanlang {

class ParsableASTNode;
using ParsableASTNodePtr = ptr<ParsableASTNode>;

class ParsableASTNode : public SourceTraceable {

public:
  /// string representation of ASTType
  static umap<ASTType, str> ASTTypeNames;

  /// operator precedence of tokens
  static umap<ASTType, int> OpPrecedence;

public:
  virtual ~ParsableASTNode() = default;

public:
  /**
   * \brief Pretty-print the AST
   * \details This requires the source code to be saved in unicode, otherwise the output will be strange. It also
   * requires the terminal to be able to print characters like '└──' and '├──'
   * */
  void printTree();

  // TODO: constraint T as a subclass of ParsableASTNode
  template<typename T = ParsableASTNode> ptr<T> get_child_at(size_t idx);
  void set_child_at(size_t idx, ptr<ParsableASTNode> node);
  void append_child(ptr<ParsableASTNode> node);
  void clear_children();
  size_t get_children_size();
  ASTType get_node_type();
  void set_node_type(ASTType node_type);
  void set_lbp(int lbp);
  int get_lbp();

  template<typename T> void set_data(T val);
  template<typename T> T get_data() const;

  vector<ParsableASTNodePtr> get_children() const;
  vector<ParsableASTNodePtr> &get_children();

protected:
  virtual str to_string(bool print_prefix = true);

private:
  void printTree(const str &prefix, bool last_child);

private:
  vector<ParsableASTNodePtr> _children{};
  ASTType _type = ASTType::INVALID;
  int _lbp = 0;

  std::variant<str, uint64_t, double> _data;
};

template<typename T> std::shared_ptr<T> ast_cast(ParsableASTNodePtr node) {
  return std::reinterpret_pointer_cast<T>(node);
}

// TODO: replace some ast_cast calls with ast_must_cast if suitable
template<typename T> std::shared_ptr<T> ast_must_cast(ParsableASTNodePtr node) {
  auto ret = std::reinterpret_pointer_cast<T>(node);
  TAN_ASSERT(ret);
  return ret;
}

}

#endif //__TAN_SRC_AST_PARSABLE_AST_NODE_H__
