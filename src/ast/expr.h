#ifndef __TAN_SRC_AST_EXPR_H__
#define __TAN_SRC_AST_EXPR_H__
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_named.h"
#include "src/ast/typed.h"

namespace tanlang {

AST_FWD_DECL(ASTType);

class Expr : public ASTBase, public Typed {
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

  uint64_t get_value() const { return _value; }
  bool is_unsigned() const { return _is_unsigned; }

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

  str get_value();

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

public:
  ASTBasePtr _referred = nullptr
};

/// make sure to sync this with BinaryOperator::BOPPrecedence
enum class BinaryOpKind {
  INVALID,      ///
  SUM,          /// +
  SUBTRACT,     /// -
  MULTIPLY,     /// *
  DIVIDE,       /// /
  MOD,          /// %
  ASSIGN,       /// =
  BAND,         /// binary and
  LAND,         /// logical and
  BOR,          /// binary or
  LOR,          /// logical or
  GT,           /// >
  GE,           /// >=
  LT,           /// <
  LE,           /// <=
  EQ,           /// ==
  NE,           /// !=
  XOR,          /// ^
  MEMBER_ACCESS, /// . or []
  CAST,         /// as
};

class BinaryOperator : public Expr {
public:
  static ptr<BinaryOperator> Create(BinaryOpKind op);
  static ptr<BinaryOperator> Create(BinaryOpKind op, const ptr<Expr> &lhs, const ptr<Expr> &rhs);

  /// binary operator precedence
  static umap<BinaryOpKind, int> BOPPrecedence;

  BinaryOperator(BinaryOpKind op);

  void set_lhs(const ptr<Expr> &lhs);
  void set_rhs(const ptr<Expr> &rhs);
  ptr<Expr> get_lhs() const;
  ptr<Expr> get_rhs() const;
  BinaryOpKind get_op() const;
  size_t get_dominant_idx() const { return _dominant_idx; }
  void set_dominant_idx(size_t idx) { _dominant_idx = idx; }

protected:
  BinaryOpKind _op;
  ptr<Expr> _lhs = nullptr;
  ptr<Expr> _rhs = nullptr;
  size_t _dominant_idx = 0;
};

class MemberAccess : public BinaryOperator {
public:
  static ptr<MemberAccess> Create();
  MemberAccess() : BinaryOperator(BinaryOpKind::MEMBER_ACCESS) {}

public:
  enum {
    MemberAccessInvalid = 0,
    MemberAccessBracket,
    MemberAccessMemberVariable,
    MemberAccessMemberFunction,
    MemberAccessDeref,
    MemberAccessEnumValue,
  } _access_type = MemberAccessInvalid;
  size_t _access_idx = (size_t) -1; /// struct member variable index
};

/// make sure to sync this with UnaryOperator::UOPPrecedence
enum class UnaryOpKind {
  INVALID,     ///
  BNOT,        /// bitwise not
  LNOT,        /// logical not
  PLUS,        /// +
  MINUS,       /// -
  ADDRESS_OF,  /// var v: int; &v
  // TODO: other unary operators
};

class UnaryOperator : public Expr {
public:
  static ptr<UnaryOperator> Create(UnaryOpKind op);
  static ptr<UnaryOperator> Create(UnaryOpKind op, const ptr<Expr> &rhs);

  /// binary operator precedence
  static umap<UnaryOpKind, int> UOPPrecedence;

  UnaryOperator(UnaryOpKind op);

  UnaryOpKind get_op() const;
  ptr<Expr> void get_rhs() const;
  void set_rhs(const ptr<Expr> &rhs);

protected:
  UnaryOpKind _op;
  ptr<Expr> _rhs = nullptr;
};

class Parenthesis : public Expr {
public:
  static ptr<Parenthesis> Create();
  Parenthesis();

  void set_sub(const ptr<Expr> &sub);

private:
  ptr<Expr> _sub = nullptr;
};

AST_FWD_DECL(FunctionDecl);

class FunctionCall : public Expr, public ASTNamed {
public:
  static ptr<FunctionCall> Create();
  FunctionCall();

public:
  vector<ptr<Expr>> _args{};
  FunctionDeclPtr _callee = nullptr;
};

}

#endif //__TAN_SRC_AST_EXPR_H__
