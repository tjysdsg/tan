#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include "base.h"
#include "src/llvm_include.h"
#include "src/ast/interface.h"

namespace tanlang {
struct Token;
enum class Ty : uint64_t;

class CompilerSession;

class Parser;

enum PrecedenceLevel {
  PREC_LOWEST = 0, //
  PREC_LITERAL = 10,      // "string" 1.0 2
  PREC_ASSIGN = 90,       // = *= /= %= += -= <<= >>= &= ^= |=
  PREC_LOGICAL_OR = 110,  // ||
  PREC_LOGICAL_AND = 120, // &&
  PREC_COMPARISON = 130,  // < <= > >= == != === !== ~=
  PREC_ISA = 132,         // isa
  PREC_RANGE = 135,       // ..< ...
  PREC_TERM = 140,        // + - | ^
  PREC_FACTOR = 150,      // * / % &
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
  XOR,       // ^

  ID, // identifiers

  /// types in tan

  TY, // type name
  PARENTHESIS, // ( )

  RET,  // return
  IF,   // if
  ELSE, // else

  NUM_LITERAL,    // int or float literal
  STRING_LITERAL, // "xxx"
  ARRAY_LITERAL,  // [1, 2, 3]
  MEMBER_ACCESS,  // struct.a

  INTRINSIC, // intrinsic functions, operators, qualifiers, etc.
  INVALID,
};

extern std::unordered_map<ASTType, std::string> ast_type_names;
// operator precedence for each token
extern std::unordered_map<ASTType, int> op_precedence;

class ASTNode {
public:
  friend class Parser;

public:
  ASTType _type = ASTType::INVALID;
  std::vector<std::shared_ptr<ASTNode>> _children{};
  int _lbp = 0;
  int _rbp = 0;
  Token *_token = nullptr;

  ASTNode() = delete;
  ASTNode(const ASTNode &) = default;
  ASTNode &operator=(const ASTNode &) = default;

  ASTNode(ASTType op, int lbp, int rbp, Token *token, size_t token_index);
  virtual ~ASTNode() = default;
  [[nodiscard]] virtual size_t parse(const std::shared_ptr<ASTNode> &left, Parser *parser);
  [[nodiscard]] virtual size_t parse(Parser *parser);
  void printTree() const;
  virtual Value *codegen(CompilerSession *compiler_session);
  virtual std::string to_string(bool print_prefix = true) const;

  virtual bool is_typed() const { return false; }

  virtual bool is_named() const { return false; }

  virtual bool is_lvalue() const { return false; };

  virtual std::string get_name() const { return ""; };

  virtual std::string get_type_name() const { return ""; };

  virtual llvm::Type *to_llvm_type(CompilerSession *) const { return nullptr; };

  virtual llvm::Value *get_llvm_value(CompilerSession *) const { return nullptr; };

protected:
  [[nodiscard]] virtual size_t led(const std::shared_ptr<ASTNode> &left, Parser *parser);
  [[nodiscard]] virtual size_t nud(Parser *parser);

private:
  void printTree(const std::string &prefix, bool last_child) const;

protected:
  bool _parsed = false;
  size_t _start_index = 0;
  size_t _end_index = 0;
  Parser *_parser = nullptr;
};

/// dummy, all literal types inherit from this class
class ASTLiteral : public ASTNode {
public:
  ASTLiteral() = delete;

  ASTLiteral(ASTType op, int lbp, int rbp, Token *token, size_t token_index) : ASTNode(op, lbp, rbp, token, token_index
  ) {}

  virtual Ty get_ty() const;

  /// literals are always rvalue
  bool is_lvalue() const override { return false; }

};

class ASTInfixBinaryOp : public ASTNode {
public:
  ASTInfixBinaryOp() = delete;
  ASTInfixBinaryOp(Token *token, size_t token_index);
protected:
  size_t led(const std::shared_ptr<ASTNode> &left, Parser *parser) override;
};

class ASTPrefix : public ASTNode {
public:
  ASTPrefix() = delete;
  ASTPrefix(Token *token, size_t token_index);
protected:
  size_t nud(Parser *parser) override;
};

class ASTReturn final : public ASTPrefix {
public:
  ASTReturn() = delete;
  ASTReturn(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTBinaryNot final : public ASTPrefix {
public:
  ASTBinaryNot() = delete;
  ASTBinaryNot(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTLogicalNot final : public ASTPrefix {
public:
  ASTLogicalNot() = delete;
  ASTLogicalNot(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTCompare final : public ASTInfixBinaryOp {
public:
  ASTCompare() = delete;
  ASTCompare(ASTType type, Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

template<typename T>
std::shared_ptr<T> ast_cast(std::shared_ptr<ASTNode> node) { return std::reinterpret_pointer_cast<T>(node); }

} // namespace tanlang

#endif /* TAN_SRC_AST_ASTNODE_H_ */
