#ifndef __TAN_SRC_AST_EXPR_H__
#define __TAN_SRC_AST_EXPR_H__
#include <utility>

#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_named.h"
#include "src/ast/typed.h"
#include "src/ast/fwd.h"

namespace tanlang {

class Expr : public ASTBase, public Typed {
protected:
  Expr(ASTNodeType type, SrcLoc loc, int bp);

public:
  virtual bool is_comptime_known() { return false; }
  virtual bool is_lvalue() { return _is_lvalue; }
  virtual void set_lvalue(bool is_lvalue) { _is_lvalue = is_lvalue; }

protected:
  bool _is_lvalue = false;
};

/**
 * \brief Compile-time expression
 * \details Compile-time expression can be a value known by itself at compile time, or the result of compile-time
 * evaluation.
 * TODO: implement this
 */
class CompTimeExpr : public Expr {
protected:
  CompTimeExpr(ASTNodeType type, SrcLoc loc, int bp);

public:
  bool is_comptime_known() override;
};

class Literal : public CompTimeExpr {
protected:
  Literal(ASTNodeType type, SrcLoc loc, int bp);
};

class BoolLiteral : public Literal {
protected:
  explicit BoolLiteral(SrcLoc loc);

public:
  static BoolLiteral *Create(SrcLoc loc, bool val);
  [[nodiscard]] bool get_value() const;

private:
  bool _value = false;
};

class IntegerLiteral : public Literal {
protected:
  explicit IntegerLiteral(SrcLoc loc);

public:
  static IntegerLiteral *Create(SrcLoc loc, uint64_t val, bool is_unsigned = false);

  [[nodiscard]] uint64_t get_value() const { return _value; }
  [[nodiscard]] bool is_unsigned() const { return _is_unsigned; }

private:
  uint64_t _value = 0;
  bool _is_unsigned = false;
};

class FloatLiteral : public Literal {
protected:
  explicit FloatLiteral(SrcLoc loc);

public:
  static FloatLiteral *Create(SrcLoc loc, double val);
  [[nodiscard]] double get_value() const;
  void set_value(double value);

private:
  double _value = 0;
};

class StringLiteral : public Literal {
protected:
  explicit StringLiteral(SrcLoc loc);

public:
  static StringLiteral *Create(SrcLoc loc, const str &val);

  [[nodiscard]] str get_value() const;
  void set_value(str val) { _value = std::move(val); }

private:
  str _value;
};

class CharLiteral : public Literal {
protected:
  explicit CharLiteral(SrcLoc loc);

public:
  static CharLiteral *Create(SrcLoc loc, uint8_t val);

  void set_value(uint8_t val);
  [[nodiscard]] uint8_t get_value() const;

private:
  uint8_t _value = 0;
};

class ArrayLiteral : public Literal {
protected:
  explicit ArrayLiteral(SrcLoc loc);

public:
  static ArrayLiteral *Create(SrcLoc loc, vector<Literal *> val);
  static ArrayLiteral *Create(SrcLoc loc);

  void set_elements(const vector<Literal *> &elements);
  [[nodiscard]] vector<Literal *> get_elements() const;

private:
  vector<Literal *> _elements{};
};

class NullPointerLiteral : public Literal {
protected:
  explicit NullPointerLiteral(SrcLoc loc);

public:
  static NullPointerLiteral *Create(SrcLoc loc);
};

class VarRef : public Expr, public ASTNamed {
protected:
  explicit VarRef(SrcLoc loc);

public:
  static VarRef *Create(SrcLoc loc, const str &name, Decl *referred);
  [[nodiscard]] Decl *get_referred() const;

private:
  Decl *_referred = nullptr;
};

enum class IdentifierType {
  INVALID, ID_VAR_DECL, ID_TYPE_DECL,
};

class Identifier : public Expr, public ASTNamed {
protected:
  explicit Identifier(SrcLoc loc);

public:
  static Identifier *Create(SrcLoc loc, const str &name);

  [[nodiscard]] IdentifierType get_id_type() const;
  void set_var_ref(VarRef *var_ref);
  void set_type_ref(ASTType *type_ref);
  [[nodiscard]] VarRef *get_var_ref() const;
  [[nodiscard]] ASTType *get_type_ref() const;
  bool is_lvalue() override;
  void set_lvalue(bool is_lvalue) override;

private:
  IdentifierType _id_type = IdentifierType::INVALID;
  union {
    VarRef *_var_ref = nullptr;
    ASTType *_type_ref;
  };
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
  BinaryOperator(BinaryOpKind op, SrcLoc loc);

public:
  static BinaryOperator *Create(BinaryOpKind op, SrcLoc loc);
  static BinaryOperator *Create(BinaryOpKind op, SrcLoc loc, Expr *lhs, Expr *rhs);

  /// binary operator precedence
  static umap<BinaryOpKind, int> BOPPrecedence;

  void set_lhs(Expr *lhs);
  void set_rhs(Expr *rhs);
  [[nodiscard]] Expr *get_lhs() const;
  [[nodiscard]] Expr *get_rhs() const;
  [[nodiscard]] BinaryOpKind get_op() const;
  [[nodiscard]] vector<ASTBase *> get_children() const override;

protected:
  BinaryOpKind _op;
  Expr *_lhs = nullptr;
  Expr *_rhs = nullptr;
};

class MemberAccess : public BinaryOperator {
protected:
  explicit MemberAccess(SrcLoc loc);

public:
  static MemberAccess *Create(SrcLoc loc);
  bool is_lvalue() override;
  void set_lvalue(bool is_lvalue) override;

public:
  enum {
    MemberAccessInvalid = 0,
    MemberAccessBracket,
    MemberAccessMemberVariable,
    MemberAccessMemberFunction,
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
  UnaryOperator(UnaryOpKind op, SrcLoc loc);

public:
  static UnaryOperator *Create(UnaryOpKind op, SrcLoc loc);
  static UnaryOperator *Create(UnaryOpKind op, SrcLoc loc, Expr *rhs);

  /// binary operator precedence
  static umap<UnaryOpKind, int> UOPPrecedence;

  [[nodiscard]] UnaryOpKind get_op() const;
  [[nodiscard]] Expr *get_rhs() const;
  void set_rhs(Expr *rhs);

  [[nodiscard]] vector<ASTBase *> get_children() const override;

protected:
  UnaryOpKind _op = UnaryOpKind::INVALID;
  Expr *_rhs = nullptr;
};

/**
 * \note Once BinaryOrUnary::set_bop() or BinaryOrUnary::set_uop() is called,
 * the object is not allowed to change anymore
 */
class BinaryOrUnary : public Expr {
protected:
  BinaryOrUnary(SrcLoc loc, int bp);

public:
  static BinaryOrUnary *Create(SrcLoc loc, int bp);

  enum BinaryOrUnaryKind {
    UNKNOWN, BINARY, UNARY,
  };

  void set_bop(BinaryOperator *bop);
  void set_uop(UnaryOperator *uop);

  [[nodiscard]] Expr *get_expr_ptr() const;
  [[nodiscard]] ASTBase *get() const override;
  [[nodiscard]] ASTType *get_type() const override;
  void set_type(ASTType *type) override;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

  bool is_lvalue() override;
  void set_lvalue(bool is_lvalue) override;

private:
  BinaryOrUnaryKind _kind = UNKNOWN;
  union {
    BinaryOperator *_bop = nullptr;
    UnaryOperator *_uop;
  };
};

class Parenthesis : public Expr {
protected:
  explicit Parenthesis(SrcLoc loc);

public:
  static Parenthesis *Create(SrcLoc loc);

  void set_sub(Expr *sub);
  [[nodiscard]] Expr *get_sub() const;
  [[nodiscard]] vector<ASTBase *> get_children() const override;

  bool is_lvalue() override;
  void set_lvalue(bool is_lvalue) override;

private:
  Expr *_sub = nullptr;
};

class FunctionCall : public Expr, public ASTNamed {
protected:
  explicit FunctionCall(SrcLoc loc);

public:
  static FunctionCall *Create(SrcLoc loc);
  [[nodiscard]] size_t get_n_args() const;
  [[nodiscard]] Expr *get_arg(size_t i) const;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

public:
  vector<Expr *> _args{};
  FunctionDecl *_callee = nullptr;
};

class Assignment : public Expr {
protected:
  explicit Assignment(SrcLoc loc);

public:
  static Assignment *Create(SrcLoc loc);

  [[nodiscard]] ASTBase *get_lhs() const;
  void set_lhs(ASTBase *lhs);
  [[nodiscard]] Expr *get_rhs() const;
  void set_rhs(Expr *rhs);

  [[nodiscard]] vector<ASTBase *> get_children() const override;

protected:
  ASTBase *_lhs = nullptr; /// lhs can be decl or expr (identifier)
  Expr *_rhs = nullptr;
};

class Cast : public Expr {
protected:
  explicit Cast(SrcLoc loc);

public:
  static Cast *Create(SrcLoc loc);
  [[nodiscard]] Expr *get_lhs() const;
  void set_lhs(Expr *lhs);
  [[nodiscard]] ASTBase *get_rhs() const;
  void set_rhs(ASTBase *rhs);

  [[nodiscard]] vector<ASTBase *> get_children() const override;

  bool is_lvalue() override;
  void set_lvalue(bool is_lvalue) override;

protected:
  Expr *_lhs = nullptr;
  ASTBase *_rhs = nullptr;
};

}

#endif //__TAN_SRC_AST_EXPR_H__
