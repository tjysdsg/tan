#ifndef __TAN_SRC_AST_EXPR_H__
#define __TAN_SRC_AST_EXPR_H__
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_named.h"

namespace tanlang {

class Expr : public ASTBase {
public:
  Expr(ASTNodeType type, int lbp) : ASTBase(type, lbp) {}
};

class Literal : public Expr {
public:
  Literal(ASTNodeType type, int lbp) : Expr(type, lbp) {}
};

class IntegerLiteral : public Literal {
public:
  static ptr<IntegerLiteral> Create(uint64_t val, bool is_unsigned = false);
  IntegerLiteral() : Literal(ASTNodeType::INTEGER_LITERAL, 0) {}

private:
  uint64_t _value = 0;
  bool _is_unsigned = false;
};

class FloatLiteral : public Literal {
public:
  static ptr<FloatLiteral> Create(double val);
  FloatLiteral() : Literal(ASTNodeType::FLOAT_LITERAL, 0) {}

private:
  double _value = 0;
};

class StringLiteral : public Literal {
public:
  static ptr<StringLiteral> Create(str_view val);
  StringLiteral() : Literal(ASTNodeType::STRING_LITERAL, 0) {}

private:
  str _value = 0;
};

class CharLiteral : public Literal {
public:
  static ptr<CharLiteral> Create(uint8_t val);
  CharLiteral() : Literal(ASTNodeType::CHAR_LITERAL, 0) {}

private:
  uint8_t _value = 0;
};

class ArrayLiteral : public Literal {
public:
  static ptr<ArrayLiteral> Create(vector<ptr<Literal>> val);
  static ptr<ArrayLiteral> Create();
  ArrayLiteral() : Literal(ASTNodeType::ARRAY_LITERAL, 0) {}

  void set_elements(const vector<ptr<Literal>> &elements);

private:
  vector<ptr<Literal>> _elements{};
};

class Identifier : public Expr, public ASTNamed {
public:
  static ptr<Identifier> Create(str_view name);
  Identifier();
};

enum BinaryOpKind {
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
};

class BinaryOperator : public Expr {
public:
  static ptr<BinaryOperator> Create(BinaryOpKind op);
  static ptr<BinaryOperator> Create(BinaryOpKind op, const ptr<Expr> &lhs, const ptr<Expr> &rhs);
  BinaryOperator(BinaryOpKind op);

  /// binary operator precedence
  static umap<BinaryOpKind, int> BOPPrecedence;

protected:
  BinaryOpKind _op;
  ptr<Expr> _lhs = nullptr;
  ptr<Expr> _rhs = nullptr;
};

}

#endif //__TAN_SRC_AST_EXPR_H__
