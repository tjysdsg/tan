#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include "base.h"
#include "src/ast/precedence.h"
#include "src/ast/ast_type.h"
#include "src/ast/parsable_ast_node.h"
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

class ASTNode : public ParsableASTNode {
public:
  /// string representation of ASTType
  static umap<ASTType, str> ast_type_names;

  /// operator precedence of tokens
  static umap<ASTType, int> op_precedence;

public:
  ASTNode() = delete;
  ASTNode(ASTType op, int lbp);
  virtual ~ASTNode() = default;

  /**
   * \brief Pretty-print the AST
   * \details This requires the source code to be saved in unicode, otherwise the output will be strange. It also
   * requires the terminal to be able to print characters like '└──' and '├──'
   * */
  void printTree();

  virtual str to_string(bool print_prefix = true);

  ASTType get_node_type();
  void set_node_type(ASTType node_type);
  int get_lbp();
  ptr<ParsableASTNode> get_child_at(size_t idx);
  void set_child_at(size_t idx, ptr<ParsableASTNode> node);
  void append_child(ptr<ParsableASTNode> node);
  void set_value(str str_value) = 0;
  void set_value(uint64_t int_value) = 0;
  void set_value(double float_value) = 0;
  uint64_t get_int_value();
  str get_str_value();
  double get_float_value();

private:
  void printTree(const str &prefix, bool last_child);

public:
  ASTType _type = ASTType::INVALID;
  ASTTyPtr _ty = nullptr;
  vector<ASTNodePtr> _children{};
  ptr<Scope> _scope = nullptr;
  str _name;
  std::variant<str, uint64_t, double> _value;

  int _lbp = 0;
  bool _parsed = false;
  bool _is_typed = false;
  bool _is_valued = false;
  bool _is_named = false;
  bool _is_external = false;
  bool _is_public = false;

  llvm::Value *_llvm_value = nullptr;
};

template<typename T> std::shared_ptr<T> ast_cast(ptr<SourceTraceable> node) {
  return std::reinterpret_pointer_cast<T>(node);
}

#undef AST_FWD_DECL

} // namespace tanlang

#endif /* TAN_SRC_AST_ASTNODE_H_ */
