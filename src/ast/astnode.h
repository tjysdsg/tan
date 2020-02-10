#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include "base.h"
#include "src/llvm_include.h"

namespace tanlang {
struct Token;
class CompilerSession;
class Parser;

enum PrecedenceLevel {
  PREC_LOWEST,
  PREC_LITERAL = 10,
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
  PROGRAM,
  FUNC_DECL,
  FUNC_CALL,
  ARG_DECL,
  VAR_DECL,

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
  INVALID,
};

extern std::unordered_map<ASTType, std::string> ast_type_names;
// operator precedence for each token
extern std::unordered_map<ASTType, int> op_precedence;

class ASTNode {
 public:
  ASTType _type = ASTType::INVALID;
  std::vector<std::shared_ptr<ASTNode>> _children{};
  int _lbp = 0;
  int _rbp = 0;
  Token *_token = nullptr;

  ASTNode() = default;
  ASTNode(const ASTNode &) = default;
  ASTNode &operator=(const ASTNode &) = default;

  ASTNode(ASTType op, int lbp, int rbp, Token *token);
  virtual ~ASTNode() = default;
  virtual void led(const std::shared_ptr<ASTNode> &left, Parser *parser);
  virtual void nud(Parser *parser);
  void printTree() const;
  virtual Value *codegen(CompilerSession *compiler_session);

 private:
  void printTree(const std::string &prefix, bool last_child) const;

 protected:
  bool _parsed = false;
  size_t _parsed_index = 0;
};

class ASTInfixBinaryOp : public ASTNode {
 public:
  explicit ASTInfixBinaryOp(Token *token);
  void led(const std::shared_ptr<ASTNode> &left, Parser *parser) override;
};

class ASTPrefix : public ASTNode {
 public:
  explicit ASTPrefix(Token *token);
  void nud(Parser *parser) override;
};

class ASTReturn final : public ASTPrefix {
 public:
  explicit ASTReturn(Token *token);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTBinaryNot final : public ASTPrefix {
 public:
  explicit ASTBinaryNot(Token *token);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTLogicalNot final : public ASTPrefix {
 public:
  explicit ASTLogicalNot(Token *token);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTCompare final : public ASTInfixBinaryOp {
 public:
  ASTCompare() = delete;
  ASTCompare(ASTType type, Token *token);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTArithmetic final : public ASTInfixBinaryOp {
 public:
  ASTArithmetic(ASTType type, Token *token);
  Value *codegen(CompilerSession *compiler_session) override;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_ASTNODE_H_ */
