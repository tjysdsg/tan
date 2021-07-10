#ifndef __TAN_SRC_AST_EXPR_H__
#define __TAN_SRC_AST_EXPR_H__
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_named.h"
#include "src/ast/typed.h"
#include "src/ast/fwd.h"

namespace tanlang {

class Expr : public ASTBase, public Typed {
protected:
  Expr(ASTNodeType type, SourceIndex loc, int bp);

public:
  virtual bool is_comptime_known() { return false; }
};

/**
 * \brief Compile-time expression
 * \details Compile-time expression can be a value known by itself at compile time, or the result of compile-time
 * evaluation.
 * TODO: implement this
 */
class CompTimeExpr : public Expr {
protected:
  CompTimeExpr(ASTNodeType type, SourceIndex loc, int bp);

public:
  bool is_comptime_known() override;
};

class Literal : public CompTimeExpr {
protected:
  Literal(ASTNodeType type, SourceIndex loc, int bp);
};

class IntegerLiteral : public Literal {
protected:
  IntegerLiteral(SourceIndex loc);

public:
  static IntegerLiteral *Create(SourceIndex loc, uint64_t val, bool is_unsigned = false);

  uint64_t get_value() const { return _value; }
  bool is_unsigned() const { return _is_unsigned; }

private:
  uint64_t _value = 0;
  bool _is_unsigned = false;
};

class FloatLiteral : public Literal {
protected:
  FloatLiteral(SourceIndex loc);

public:
  static FloatLiteral *Create(SourceIndex loc, double val);
  double get_value() const;
  void set_value(double value);

private:
  double _value = 0;
};

class StringLiteral : public Literal {
protected:
  StringLiteral(SourceIndex loc);

public:
  static StringLiteral *Create(SourceIndex loc, const str &val);

  str get_value() const;
  void set_value(str val) { _value = val; }

private:
  str _value;
};

class CharLiteral : public Literal {
protected:
  CharLiteral(SourceIndex loc);

public:
  static CharLiteral *Create(SourceIndex loc, uint8_t val);

  void set_value(uint8_t val);
  uint8_t get_value() const;

private:
  uint8_t _value = 0;
};

class ArrayLiteral : public Literal {
protected:
  ArrayLiteral(SourceIndex loc);

public:
  static ArrayLiteral *Create(SourceIndex loc, vector<Literal *> val);
  static ArrayLiteral *Create(SourceIndex loc);

  void set_elements(const vector<Literal *> &elements);
  vector<Literal *> get_elements() const;

private:
  vector<Literal *> _elements{};
};

class Identifier : public Expr, public ASTNamed {
protected:
  Identifier(SourceIndex loc);

public:
  static Identifier *Create(SourceIndex loc, const str &name);

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
protected:
  BinaryOperator(BinaryOpKind op, SourceIndex loc);

public:
  static BinaryOperator *Create(BinaryOpKind op, SourceIndex loc);
  static BinaryOperator *Create(BinaryOpKind op, SourceIndex loc, Expr *lhs, Expr *rhs);

  /// binary operator precedence
  static umap<BinaryOpKind, int> BOPPrecedence;

  void set_lhs(Expr *lhs);
  void set_rhs(Expr *rhs);
  Expr *get_lhs() const;
  Expr *get_rhs() const;
  BinaryOpKind get_op() const;
  size_t get_dominant_idx() const { return _dominant_idx; }
  void set_dominant_idx(size_t idx) { _dominant_idx = idx; }

  vector<ASTBase *> get_children() const override;

public:
  size_t _dominant_idx = 0;

protected:
  BinaryOpKind _op;
  Expr *_lhs = nullptr;
  Expr *_rhs = nullptr;
};

class MemberAccess : public BinaryOperator {
protected:
  MemberAccess(SourceIndex loc);

public:
  static MemberAccess *Create(SourceIndex loc);

public:
  enum {
    MemberAccessInvalid = 0,
    MemberAccessBracket,
    MemberAccessMemberVariable,
    MemberAccessMemberFunction,
    // TODO: remove this
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
  PTR_DEREF,   /// poitner dereference
};

class UnaryOperator : public Expr {
protected:
  UnaryOperator(UnaryOpKind op, SourceIndex loc);

public:
  static UnaryOperator *Create(UnaryOpKind op, SourceIndex loc);
  static UnaryOperator *Create(UnaryOpKind op, SourceIndex loc, Expr *rhs);

  /// binary operator precedence
  static umap<UnaryOpKind, int> UOPPrecedence;

  UnaryOpKind get_op() const;
  Expr *get_rhs() const;
  void set_rhs(Expr *rhs);

  vector<ASTBase *> get_children() const override;

protected:
  UnaryOpKind _op;
  Expr *_rhs = nullptr;
};

/**
 * \note Once BinaryOrUnary::set_bop() or BinaryOrUnary::set_uop() is called,
 * the object is not allowed to change anymore
 */
class BinaryOrUnary : public Expr {
protected:
  BinaryOrUnary(SourceIndex loc, int bp);

public:
  static BinaryOrUnary *Create(SourceIndex loc, int bp);

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

  vector<ASTBase *> get_children() const override;

private:
  BinaryOrUnaryKind _kind = UNKNOWN;
  union {
    BinaryOperator *_bop;
    UnaryOperator *_uop;
  };
};

class Parenthesis : public Expr {
protected:
  Parenthesis(SourceIndex loc);

public:
  static Parenthesis *Create(SourceIndex loc);

  void set_sub(Expr *sub);
  Expr *get_sub() const;

  vector<ASTBase *> get_children() const override;

private:
  Expr *_sub = nullptr;
};

class FunctionCall : public Expr, public ASTNamed {
protected:
  FunctionCall(SourceIndex loc);

public:
  static FunctionCall *Create(SourceIndex loc);
  size_t get_n_args() const;
  Expr *get_arg(size_t i) const;

  vector<ASTBase *> get_children() const override;

public:
  vector<Expr *> _args{};
  FunctionDecl *_callee = nullptr;
};

class Assignment : public Expr {
protected:
  Assignment(SourceIndex loc);

public:
  static Assignment *Create(SourceIndex loc);

  ASTBase *get_lhs() const;
  void set_lhs(ASTBase *lhs);
  Expr *get_rhs() const;
  void set_rhs(Expr *rhs);

  vector<ASTBase *> get_children() const override;

protected:
  ASTBase *_lhs = nullptr; /// lhs can be decl or expr (identifier)
  Expr *_rhs = nullptr;
};

class Cast : public Expr {
protected:
  Cast(SourceIndex loc);

public:
  static Cast *Create(SourceIndex loc);
  Expr *get_lhs() const;
  void set_lhs(Expr *lhs);
  ASTBase *get_rhs() const;
  void set_rhs(ASTBase *rhs);

  vector<ASTBase *> get_children() const override;

protected:
  Expr *_lhs = nullptr;
  ASTBase *_rhs = nullptr;
};

}

#endif //__TAN_SRC_AST_EXPR_H__
