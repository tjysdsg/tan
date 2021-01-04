#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include "base.h"
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

enum PrecedenceLevel {
  PREC_LOWEST = 0,        //
  PREC_LITERAL = 10,      // "string" 1.0 2
  PREC_ASSIGN = 90,       // = *= /= %= += -= <<= >>= &= ^= |=
  PREC_LOGICAL_OR = 110,  // ||
  PREC_LOGICAL_AND = 120, // &&
  PREC_COMPARISON = 130,  // < <= > >= == != === !== ~=
  PREC_RANGE = 135,       // ..< ...
  PREC_TERM = 140,        // + - | ^
  PREC_FACTOR = 150,      // * / % &
  PREC_CAST = 155,        // as
  PREC_SHIFT = 160,       // << >>
  PREC_UNARY = 170,       // + - ! ~
  PREC_CALL = 200,        // . ( [
  PREC_HIGHEST = 500,
};

enum class ASTType {
  PROGRAM,
  FUNC_DECL,
  FUNC_CALL,
  ARG_DECL,
  VAR_DECL,
  STRUCT_DECL,
  ENUM_DECL,
  ENUM_VAL,
  STATEMENT,   /// statement or compound statements
  SUM,         /// +
  SUBTRACT,    /// -
  MULTIPLY,    /// *
  DIVIDE,      /// /
  MOD,         /// %
  ASSIGN,      /// =
  BAND,        /// binary and
  LAND,        /// logical and
  BOR,         /// binary or
  LOR,         /// logical or
  BNOT,        /// bitwise not
  LNOT,        /// logical not
  GT,          /// >
  GE,          /// >=
  LT,          /// <
  LE,          /// <=
  EQ,          /// ==
  NE,          /// !=
  XOR,         /// ^
  ADDRESS_OF,  /// &
  CAST,        /// as
  ID,          /// identifiers
  LOOP,        /// for, while, ...
  CONTINUE,    /// continue
  BREAK,       /// break
  TY,          /// type name
  PARENTHESIS, /// ()
  RET,         /// return
  IF,          /// if
  ELSE,        /// else
  IMPORT,      /// import

  // types in tan
  NUM_LITERAL,    /// int or float literal
  CHAR_LITERAL,   /// 's'
  STRING_LITERAL, /// "xxx"
  ARRAY_LITERAL,  /// [1, 2, 3]
  MEMBER_ACCESS,  /// struct.a
  INTRINSIC, /// intrinsic functions, operators, qualifiers, etc.
  INVALID,
};

class ASTNode {
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

public:
  virtual str to_string(bool print_prefix = true);

private:
  void printTree(const str &prefix, bool last_child);

public:
  ASTType _type = ASTType::INVALID;
  vector<ASTNodePtr> _children{};
  int _lbp = 0;
  Token *_token = nullptr;
  llvm::Value *_llvm_value = nullptr;
  ASTTyPtr _ty = nullptr;
  str _name;
  bool _is_typed = false;
  bool _is_valued = false;
  bool _is_named = false;
  bool _parsed = false;
  bool _is_external = false;
  bool _is_public = false;
  size_t _start_index = 0;
  size_t _end_index = 0;
  ptr<Scope> _scope = nullptr;
  std::variant<str, uint64_t, double> _value;
  size_t _dominant_idx = 0;
};

template<typename T> std::shared_ptr<T> ast_cast(ASTNodePtr node) { return std::reinterpret_pointer_cast<T>(node); }

#undef AST_FWD_DECL

} // namespace tanlang

#endif /* TAN_SRC_AST_ASTNODE_H_ */
