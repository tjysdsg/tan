#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include "base.h"
#include "src/llvm_include.h"

namespace tanlang {
struct Token;
enum class Ty : uint64_t;

class CompilerSession;

class ASTTy;

class Parser;

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
  PROGRAM, FUNC_DECL, FUNC_CALL, ARG_DECL, VAR_DECL, STRUCT_DECL,

  STATEMENT, // statement or compound statements
  SUM,       // +
  SUBTRACT,  // -
  MULTIPLY,  // *
  DIVIDE,    // /
  MOD,       // %
  ASSIGN,    // =
  BAND,      // binary and
  LAND,      // logical and
  BOR,       // binary or
  LOR,       // logical or
  BNOT,      // binary not
  LNOT,      // logical not
  GT,        // >
  GE,        // >=
  LT,        // <
  LE,        // <=
  EQ,        // ==
  NE,        // !=
  XOR,       // ^

  CAST, // as

  ID, // identifiers
  LOOP, // for, while, ...

  TY, // type name
  PARENTHESIS, // ( )

  RET,  // return
  IF,   // if
  ELSE, // else
  IMPORT, // import

  /// types in tan

  NUM_LITERAL,    // int or float literal
  CHAR_LITERAL,   // 's'
  STRING_LITERAL, // "xxx"
  ARRAY_LITERAL,  // [1, 2, 3]
  MEMBER_ACCESS,  // struct.a

  INTRINSIC, // intrinsic functions, operators, qualifiers, etc.
  INVALID,
};

/// get string representation of ASTType
extern std::unordered_map<ASTType, std::string> ast_type_names;
/// operator precedence for tokens
extern std::unordered_map<ASTType, int> op_precedence;

class ASTNode;

using ASTNodePtr = std::shared_ptr<ASTNode>;

class ASTNode {
public:
  friend class Parser;

public:
  ASTType _type = ASTType::INVALID;
  std::vector<ASTNodePtr> _children{};
  int _lbp = 0;
  int _rbp = 0;
  Token *_token = nullptr;

  ASTNode() = delete;
  ASTNode(const ASTNode &) = default;
  ASTNode &operator=(const ASTNode &) = default;
  ASTNode(ASTType op, int lbp, int rbp, Token *token, size_t token_index);
  virtual ~ASTNode() = default;
  [[nodiscard]] virtual size_t parse(const ASTNodePtr &left, Parser *parser, CompilerSession *cs);
  [[nodiscard]] virtual size_t parse(Parser *parser, CompilerSession *cs);
  void printTree() const;
  virtual Value *codegen(CompilerSession *compiler_session);
  virtual std::string to_string(bool print_prefix = true) const;
  virtual bool is_typed() const { return false; }
  virtual bool is_named() const { return false; }
  virtual bool is_lvalue() const { return false; };
  virtual std::string get_name() const { return {}; };
  virtual std::string get_type_name() const { return {}; };
  virtual std::shared_ptr<ASTTy> get_ty() const { return nullptr; };
  virtual llvm::Type *to_llvm_type(CompilerSession *) const { return nullptr; };
  virtual llvm::Metadata *to_llvm_meta(CompilerSession *) const { return nullptr; };
  virtual llvm::Value *get_llvm_value(CompilerSession *) const { return nullptr; };

  /**
   * \brief Get original source for a AST node.
   * */
  std::string get_src() const;

protected:
  [[nodiscard]] virtual size_t led(const ASTNodePtr &left);
  [[nodiscard]] virtual size_t nud();

private:
  void printTree(const std::string &prefix, bool last_child) const;

protected:
  bool _parsed = false;
  size_t _start_index = 0;
  size_t _end_index = 0;
  Parser *_parser = nullptr;
  CompilerSession *_cs = nullptr;
};

template<typename T> std::shared_ptr<T> ast_cast(ASTNodePtr node) { return std::reinterpret_pointer_cast<T>(node); }

} // namespace tanlang

#endif /* TAN_SRC_AST_ASTNODE_H_ */
