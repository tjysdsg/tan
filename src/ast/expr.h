#ifndef __TAN_SRC_AST_EXPR_H__
#define __TAN_SRC_AST_EXPR_H__
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_named.h"
#include "src/ast/typed.h"
#include "src/ast/fwd.h"

namespace tanlang {

class Expr : public ASTBase, public Typed {
public:
  Expr(ASTNodeType type, int lbp) : ASTBase(type, lbp) {}
  virtual bool is_comptime_known() { return false; }
};

/**
 * \brief Compile-time expression
 * \details Compile-time expression can be a value known by itself at compile time, or the result of compile-time
 * evaluation.
 * TODO: implement this
 */
class CompTimeExpr : public Expr {
public:
  CompTimeExpr(ASTNodeType type, int lbp) : Expr(type, lbp) {}
  bool is_comptime_known() override;
};

class Literal : public CompTimeExpr {
public:
  Literal(ASTNodeType type, int lbp) : CompTimeExpr(type, lbp) {}
};

class IntegerLiteral : public Literal {
public:
  static IntegerLiteral *Create(uint64_t val, bool is_unsigned = false);
  IntegerLiteral() : Literal(ASTNodeType::INTEGER_LITERAL, 0) {}

  uint64_t get_value() const { return _value; }
  bool is_unsigned() const { return _is_unsigned; }

private:
  uint64_t _value = 0;
  bool _is_unsigned = false;
};

class FloatLiteral : public Literal {
public:
  static FloatLiteral *Create(double val);
  FloatLiteral() : Literal(ASTNodeType::FLOAT_LITERAL, 0) {}
  double get_value() const;
  void set_value(double value);

private:
  double _value = 0;
};

class StringLiteral : public Literal {
public:
  static StringLiteral *Create(const str &val);
  StringLiteral();

  str get_value() const;
  void set_value(str val) { _value = val; }

private:
  str _value;
};

class CharLiteral : public Literal {
public:
  static CharLiteral *Create(uint8_t val);
  CharLiteral() : Literal(ASTNodeType::CHAR_LITERAL, 0) {}

  void set_value(uint8_t val);
  uint8_t get_value() const;

private:
  uint8_t _value = 0;
};

class ArrayLiteral : public Literal {
public:
  static ArrayLiteral *Create(vector<Literal *> val);
  static ArrayLiteral *Create();
  ArrayLiteral() : Literal(ASTNodeType::ARRAY_LITERAL, 0) {}

  void set_elements(const vector<Literal *> &elements);
  vector<Literal *> get_elements() const;

private:
  vector<Literal *> _elements{};
};

class Identifier : public Expr, public ASTNamed {
public:
  static Identifier *Create(const str &name);
  Identifier();

public:
  Decl *_referred = nullptr;
};

/// make sure to sync this with BinaryOperator::BOPPrecedence
enum class BinaryOpKind {
  INVALID,      ///
  SUM,          /// +
  SUBTRACT,     /// -
  MULTIPLY,     /// *
  DIVIDE,       /// /
  MOD,          /// %
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
};

class BinaryOperator : public Expr {
public:
  static BinaryOperator *Create(BinaryOpKind op);
  static BinaryOperator *Create(BinaryOpKind op, Expr *lhs, Expr *rhs);

  /// binary operator precedence
  static umap<BinaryOpKind, int> BOPPrecedence;

  BinaryOperator(BinaryOpKind op);

  void set_lhs(Expr *lhs);
  void set_rhs(Expr *rhs);
  Expr *get_lhs() const;
  Expr *get_rhs() const;
  BinaryOpKind get_op() const;
  size_t get_dominant_idx() const { return _dominant_idx; }
  void set_dominant_idx(size_t idx) { _dominant_idx = idx; }

public:
  size_t _dominant_idx = 0;

protected:
  BinaryOpKind _op;
  Expr *_lhs = nullptr;
  Expr *_rhs = nullptr;
};

class MemberAccess : public BinaryOperator {
public:
  static MemberAccess *Create();
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
  static UnaryOperator *Create(UnaryOpKind op);
  static UnaryOperator *Create(UnaryOpKind op, Expr *rhs);

  /// binary operator precedence
  static umap<UnaryOpKind, int> UOPPrecedence;

  UnaryOperator(UnaryOpKind op);

  UnaryOpKind get_op() const;
  Expr *get_rhs() const;
  void set_rhs(Expr *rhs);

protected:
  UnaryOpKind _op;
  Expr *_rhs = nullptr;
};

/**
 * \note Once BinaryOrUnary::set_bop() or BinaryOrUnary::set_uop() is called,
 * the object is not allowed to change anymore
 */
class BinaryOrUnary : public Expr {
public:
  BinaryOrUnary(int lbp);
  static BinaryOrUnary *Create(int lbp);

  enum BinaryOrUnaryKind {
    UNKNOWN, BINARY, UNARY,
  };

  BinaryOrUnaryKind get_kind() const;
  BinaryOperator *get_bop() const;
  void set_bop(BinaryOperator *bop);
  UnaryOperator *get_uop() const;
  void set_uop(UnaryOperator *uop);

  Expr *get_generic_ptr() const;
  ASTBase *get() const override;
  ASTType *get_type() const override;
  void set_type(ASTType *type) override;

private:
  BinaryOrUnaryKind _kind = UNKNOWN;
  union {
    BinaryOperator *_bop;
    UnaryOperator *_uop;
  };
};

class Parenthesis : public Expr {
public:
  static Parenthesis *Create();
  Parenthesis();

  void set_sub(Expr *sub);
  Expr *get_sub() const;

private:
  Expr *_sub = nullptr;
};

class FunctionCall : public Expr, public ASTNamed {
public:
  static FunctionCall *Create();
  FunctionCall();

public:
  vector<Expr *> _args{};
  FunctionDecl *_callee = nullptr;
};

class Assignment : public Expr {
public:
  static Assignment *Create();
  Assignment();
  ASTBase *get_lhs() const;
  void set_lhs(ASTBase *lhs);
  Expr *get_rhs() const;
  void set_rhs(Expr *rhs);

protected:
  ASTBase *_lhs = nullptr; /// lhs can be decl or expr (identifier)
  Expr *_rhs = nullptr;
};

class Cast : public Expr {
public:
  static Cast *Create();
  Cast();
  Expr *get_lhs() const;
  void set_lhs(Expr *lhs);
  ASTBase *get_rhs() const;
  void set_rhs(ASTBase *rhs);

  ASTType *get_dest_type() const;
  void set_dest_type(ASTType *dest_type);

protected:
  Expr *_lhs = nullptr;
  ASTBase *_rhs = nullptr;
  ASTType *_dest_type = nullptr;
};

}

#endif //__TAN_SRC_AST_EXPR_H__
