#ifndef TAN_SRC_AST_ASTNODE_H_
#define TAN_SRC_AST_ASTNODE_H_
#include <vector>
#include "base.h"
#include "parser.h"

namespace tanlang {
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
  PREC_CALL = 200         // . ( [
};

enum class ASTType {
  PROGRAM,

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
  XOR,

  NUM_LITERAL,
  STRING_LITERAL,
  EOF_,
};

extern std::unordered_map<ASTType, std::string> ast_type_names;

// operator precedence for each token
extern std::unordered_map<ASTType, int> op_precedence;

class ASTNode {
 public:
  ASTType _op = ASTType::PROGRAM;
  int _associativity{}; // 0 left, 1 non-left
  std::vector<ASTNode *> _children{};
  int _lbp = 0;
  int _rbp = 0;

  ASTNode() = default;

  ASTNode(ASTType op, int associativity, int lbp,
          int rbp) : _op(op), _associativity(associativity), _lbp(lbp), _rbp(rbp) {};

  virtual ~ASTNode() {
      for (auto *&c : _children) {
          delete c;
          c = nullptr;
      }
  };

  [[nodiscard]] virtual int get_ivalue() const {
      assert(false);
      return 0;
  }

  [[nodiscard]] virtual float get_fvalue() const {
      assert(false);
      return 0;
  }

  [[nodiscard]] virtual std::string get_svalue() const {
      assert(false);
      return "";
  }

  [[nodiscard]] virtual ASTNode *led(ASTNode *left, Parser *parser) {
      assert(false);
      return nullptr;
  }

  [[nodiscard]] virtual ASTNode *nud(Parser *parser) {
      assert(false);
      return nullptr;
  }

  virtual void add(ASTNode *c) {
      _children.emplace_back(c);
  }

  void printTree();

  void printSubtree(const std::string &prefix);
};

class ASTInfixBinaryOp : public ASTNode {
 public:
  ASTInfixBinaryOp() : ASTNode(ASTType::EOF_, 0, op_precedence[ASTType::EOF_], 0) {
  };

  [[nodiscard]] ASTNode *led(ASTNode *left, Parser *parser) override;

  [[nodiscard]] ASTNode *nud(Parser *parser) override;
};

class ASTNumberLiteral final : public ASTNode {
 public:
  ASTNumberLiteral(const std::string &str, bool is_float) : ASTNode(ASTType::NUM_LITERAL, 1,
                                                                    op_precedence[ASTType::NUM_LITERAL], 0) {
      _is_float = is_float;
      if (is_float) {
          _fvalue = std::stof(str);
      } else {
          _ivalue = std::stoi(str);
      }
  };

  [[nodiscard]] ASTNode *nud(Parser *parser) override {
      return this;
  }

  [[nodiscard]] bool is_float() const {
      return _is_float;
  }

  [[nodiscard]] int get_ivalue() const override {
      assert(!_is_float);
      return _ivalue;
  }

  [[nodiscard]] float get_fvalue() const override {
      assert(_is_float);
      return _fvalue;
  }

 private:
  bool _is_float = false;
  union {
    int _ivalue;
    float _fvalue;
  };
};

class ASTPrefix : public ASTNode {
 public:
  ASTPrefix() : ASTNode(ASTType::EOF_, 1, op_precedence[ASTType::EOF_], 0) {}

  [[nodiscard]] ASTNode *nud(Parser *parser) override {
      auto *n = parser->next_expression(_lbp);
      if (!n) {
          // TODO: report error
      } else {
          _children.emplace_back(n);
      }
      return this;
  }
};

class ASTBinaryNot final : public ASTPrefix {
 public:
  ASTBinaryNot() : ASTPrefix() {
      _op = ASTType::BNOT;
      _lbp = op_precedence[_op];
  }
};

class ASTLogicalNot final : public ASTPrefix {
 public:
  ASTLogicalNot() : ASTPrefix() {
      _op = ASTType::LNOT;
      _lbp = op_precedence[_op];
  }
};

class ASTStringLiteral final : public ASTNode {
 public:
  explicit ASTStringLiteral(std::string str) : ASTNode(ASTType::STRING_LITERAL, 1,
                                                       op_precedence[ASTType::STRING_LITERAL],
                                                       0) {
      _svalue = std::move(str);
  }

  [[nodiscard]] std::string get_svalue() const override {
      return _svalue;
  }

 private:
  std::string _svalue;
};

class ASTSum final : public ASTInfixBinaryOp {
 public:
  ASTSum() : ASTInfixBinaryOp() {
      _op = ASTType::SUM;
      _lbp = op_precedence[ASTType::SUM];
  };
};

class ASTSubtract final : public ASTInfixBinaryOp {
 public:
  ASTSubtract() : ASTInfixBinaryOp() {
      _op = ASTType::SUBTRACT;
      _lbp = op_precedence[ASTType::SUBTRACT];
  };
};

class ASTMultiply final : public ASTInfixBinaryOp {
 public:
  ASTMultiply() : ASTInfixBinaryOp() {
      _op = ASTType::MULTIPLY;
      _lbp = op_precedence[ASTType::MULTIPLY];
  };
};

class ASTDivide final : public ASTInfixBinaryOp {
 public:
  ASTDivide() : ASTInfixBinaryOp() {
      _op = ASTType::DIVIDE;
      _lbp = op_precedence[ASTType::DIVIDE];
  }
};

}

#endif /* TAN_SRC_AST_ASTNODE_H_ */
