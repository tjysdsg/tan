#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include <vector>
#include "base.h"
#include "parser.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

namespace tanlang {

enum PrecedenceLevel {
  PREC_LOWEST,
  PREC_KEYWORD = 5, // FIXME?
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
  PREC_CALL = 200         // . ( [
};

enum class ASTType {
  PROGRAM,
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

  //
      RET,

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
  int _associativity{}; // 0 left, 1 non-left
  std::vector<std::shared_ptr<ASTNode>> _children{};
  int _lbp = 0;
  int _rbp = 0;

  ASTNode() = default;
  ASTNode(ASTType op, int associativity, int lbp, int rbp);
  virtual ~ASTNode() = default;
  [[nodiscard]] virtual int get_ivalue() const;
  [[nodiscard]] virtual float get_fvalue() const;
  [[nodiscard]] virtual std::string get_svalue() const;
  virtual void led(const std::shared_ptr<ASTNode> &left, Parser *parser);
  virtual void nud(Parser *parser);
  virtual void add(ASTNode *c);
  void printTree() const;
  void printSubtree(const std::string &prefix) const;
  [[noreturn]]virtual Value *codegen(ParserContext *parser_context) {
    UNUSED(parser_context);
    assert(false);
  }
};

class ASTProgram : public ASTNode {
 public:
  ASTProgram();
  Value *codegen(ParserContext *parser_context) override;
  void nud(Parser *parser) override;
};

class ASTStatement : public ASTNode {
 public:
  ASTStatement();
  explicit ASTStatement(bool is_compound);
  Value *codegen(ParserContext *parser_context) override;
  void nud(Parser *parser) override;
 public:
  bool _is_compound = false;
};

class ASTInfixBinaryOp : public ASTNode {
 public:
  ASTInfixBinaryOp();
  void led(const std::shared_ptr<ASTNode> &left, Parser *parser) override;
};

class ASTNumberLiteral final : public ASTNode {
 public:
  ASTNumberLiteral(const std::string &str, bool is_float);
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
  ASTPrefix();
  void nud(Parser *parser) override;
};

class ASTReturn final : public ASTPrefix {
 public:
  ASTReturn();
  Value *codegen(ParserContext *parser_context) override;
};

class ASTBinaryNot final : public ASTPrefix {
 public:
  ASTBinaryNot();
  Value *codegen(ParserContext *parser_context) override;
};

class ASTLogicalNot final : public ASTPrefix {
 public:
  ASTLogicalNot();
  Value *codegen(ParserContext *parser_context) override;
};

class ASTStringLiteral final : public ASTNode {
 public:
  explicit ASTStringLiteral(std::string str);
  [[nodiscard]] std::string get_svalue() const override;
  // Value *codegen(ParserContext *parser_context) override;

 private:
  std::string _svalue;
};

class ASTCompare final : public ASTInfixBinaryOp {
 public:
  ASTCompare() = delete;
  explicit ASTCompare(ASTType type);
  Value *codegen(ParserContext *parser_context) override;
};

class ASTSum final : public ASTInfixBinaryOp {
 public:
  ASTSum();
  Value *codegen(ParserContext *parser_context) override;
};

class ASTSubtract final : public ASTInfixBinaryOp {
 public:
  ASTSubtract();
  Value *codegen(ParserContext *parser_context) override;
};

class ASTMultiply final : public ASTInfixBinaryOp {
 public:
  ASTMultiply();
  Value *codegen(ParserContext *parser_context) override;
};

class ASTDivide final : public ASTInfixBinaryOp {
 public:
  ASTDivide();
  Value *codegen(ParserContext *parser_context) override;
};

}

#endif /* TAN_SRC_AST_ASTNODE_H_ */
