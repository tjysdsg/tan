#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include "base.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

namespace llvm {
class Value;
}

namespace tanlang {
struct Token;
struct ParserContext;
class Parser;
using llvm::Value;

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
  FUNC,
  ARG_DECL,
  VAR_DECL,

  STATEMENT, // statement or compound statements
  SUM,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  MOD,
  ASSIGN,
  BAND, // binary and
  LAND, // logical and
  BOR,  // binary or
  LOR,  // logical or
  BNOT, // binary not
  LNOT, // logical not
  GT, // >
  GE, // >=
  LT, // <
  LE, // <=
  XOR,

  ID, // identifiers
  TYPENAME,

  PARENTHESIS, // ( )

  RET,
  IF,
  ELSE,

  NUM_LITERAL,
  STRING_LITERAL,
  INVALID,
};

extern std::unordered_map<ASTType, std::string> ast_type_names;
// operator precedence for each token
extern std::unordered_map<ASTType, int> op_precedence;

class ASTNode {
 public:
  ASTType _op = ASTType::INVALID;
  std::vector<std::shared_ptr<ASTNode>> _children{};
  int _lbp = 0;
  int _rbp = 0;
  Token *_token = nullptr;

  ASTNode() = default;
  ASTNode(const ASTNode &) = default;
  ASTNode &operator=(const ASTNode &) = default;

  ASTNode(ASTType op, int lbp, int rbp, Token *token);
  virtual ~ASTNode() = default;
  [[nodiscard]] virtual int get_ivalue() const;
  [[nodiscard]] virtual float get_fvalue() const;
  [[nodiscard]] virtual std::string get_svalue() const;
  virtual void led(const std::shared_ptr<ASTNode> &left, Parser *parser);
  virtual void nud(Parser *parser);
  void printTree() const;
  virtual Value *codegen(ParserContext *parser_context);
  [[noreturn]]void report_error();
 private:
  void printTree(const std::string &prefix, bool last_child) const;
};

class ASTTypeName : public ASTNode {
 public:
  ASTTypeName() = delete;
  explicit ASTTypeName(Token *token);
  void nud(Parser *parser) override;

 public:
  std::string _name;
};

class ASTInfixBinaryOp : public ASTNode {
 public:
  explicit ASTInfixBinaryOp(Token *token);
  void led(const std::shared_ptr<ASTNode> &left, Parser *parser) override;
};

class ASTNumberLiteral final : public ASTNode {
 public:
  ASTNumberLiteral(const std::string &str, bool is_float, Token *token);
  void nud(Parser *parser) override;
  [[nodiscard]] bool is_float() const;
  [[nodiscard]] int get_ivalue() const override;
  [[nodiscard]] float get_fvalue() const override;
  Value *codegen(ParserContext *parser_context) override;

 private:
  bool _is_float = false;
  union {
    int _ivalue;
    float _fvalue;
  };
};

class ASTPrefix : public ASTNode {
 public:
  explicit ASTPrefix(Token *token);
  void nud(Parser *parser) override;
};

class ASTReturn final : public ASTPrefix {
 public:
  explicit ASTReturn(Token *token);
  Value *codegen(ParserContext *parser_context) override;
};

class ASTBinaryNot final : public ASTPrefix {
 public:
  explicit ASTBinaryNot(Token *token);
  Value *codegen(ParserContext *parser_context) override;
};

class ASTLogicalNot final : public ASTPrefix {
 public:
  explicit ASTLogicalNot(Token *token);
  Value *codegen(ParserContext *parser_context) override;
};

class ASTStringLiteral final : public ASTNode {
 public:
  explicit ASTStringLiteral(std::string str, Token *token);
  [[nodiscard]] std::string get_svalue() const override;
  // Value *codegen(ParserContext *parser_context) override;

 private:
  std::string _svalue;
};

class ASTCompare final : public ASTInfixBinaryOp {
 public:
  ASTCompare() = delete;
  explicit ASTCompare(ASTType type, Token *token);
  Value *codegen(ParserContext *parser_context) override;
};

class ASTArithmetic final : public ASTInfixBinaryOp {
 public:
  explicit ASTArithmetic(ASTType type, Token *token);
  Value *codegen(ParserContext *parser_context) override;
};

}

#endif /* TAN_SRC_AST_ASTNODE_H_ */
