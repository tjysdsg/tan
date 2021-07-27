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

class BoolLiteral : public Literal {
protected:
  explicit BoolLiteral(SourceIndex loc);

public:
  static BoolLiteral *Create(SourceIndex loc, bool val);
  [[nodiscard]] bool get_value() const;

private:
  bool _value = false;
};

class IntegerLiteral : public Literal {
protected:
  explicit IntegerLiteral(SourceIndex loc);

public:
  static IntegerLiteral *Create(SourceIndex loc, uint64_t val, bool is_unsigned = false);

  [[nodiscard]] uint64_t get_value() const { return _value; }
  [[nodiscard]] bool is_unsigned() const { return _is_unsigned; }

private:
  uint64_t _value = 0;
  bool _is_unsigned = false;
};

class FloatLiteral : public Literal {
protected:
  explicit FloatLiteral(SourceIndex loc);

public:
  static FloatLiteral *Create(SourceIndex loc, double val);
  [[nodiscard]] double get_value() const;
  void set_value(double value);

private:
  double _value = 0;
};

class StringLiteral : public Literal {
protected:
  explicit StringLiteral(SourceIndex loc);

public:
  static StringLiteral *Create(SourceIndex loc, const str &val);

  [[nodiscard]] str get_value() const;
  void set_value(str val) { _value = std::move(val); }

private:
  str _value;
};

class CharLiteral : public Literal {
protected:
  explicit CharLiteral(SourceIndex loc);

public:
  static CharLiteral *Create(SourceIndex loc, uint8_t val);

  void set_value(uint8_t val);
  [[nodiscard]] uint8_t get_value() const;

private:
  uint8_t _value = 0;
};

class ArrayLiteral : public Literal {
protected:
  explicit ArrayLiteral(SourceIndex loc);

public:
  static ArrayLiteral *Create(SourceIndex loc, vector<Literal *> val);
  static ArrayLiteral *Create(SourceIndex loc);

  void set_elements(const vector<Literal *> &elements);
  [[nodiscard]] vector<Literal *> get_elements() const;

private:
  vector<Literal *> _elements{};
};

class NullPointerLiteral : public Literal {
protected:
  explicit NullPointerLiteral(SourceIndex loc);

public:
  static NullPointerLiteral *Create(SourceIndex loc);
};

class VarRef : public Expr, public ASTNamed {
protected:
  explicit VarRef(SourceIndex loc);

public:
  static VarRef *Create(SourceIndex loc, const str &name, Decl *referred);
  [[nodiscard]] Decl *get_referred() const;

private:
  Decl *_referred = nullptr;
};

enum class IdentifierType {
  INVALID, ID_VAR_DECL, ID_TYPE_DECL,
};

class Identifier : public Expr, public ASTNamed {
protected:
  explicit Identifier(SourceIndex loc);

public:
  static Identifier *Create(SourceIndex loc, const str &name);

  [[nodiscard]] IdentifierType get_id_type() const;
  void set_var_ref(VarRef *var_ref);
  void set_type_ref(ASTType *type_ref);
  [[nodiscard]] VarRef *get_var_ref() const;
  [[nodiscard]] ASTType *get_type_ref() const;

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
  BinaryOperator(BinaryOpKind op, SourceIndex loc);

public:
  static BinaryOperator *Create(BinaryOpKind op, SourceIndex loc);
  static BinaryOperator *Create(BinaryOpKind op, SourceIndex loc, Expr *lhs, Expr *rhs);

  /// binary operator precedence
  static umap<BinaryOpKind, int> BOPPrecedence;

  void set_lhs(Expr *lhs);
  void set_rhs(Expr *rhs);
  [[nodiscard]] Expr *get_lhs() const;
  [[nodiscard]] Expr *get_rhs() const;
  [[nodiscard]] BinaryOpKind get_op() const;
  [[nodiscard]] size_t get_dominant_idx() const { return _dominant_idx; }
  void set_dominant_idx(size_t idx) { _dominant_idx = idx; }

  [[nodiscard]] vector<ASTBase *> get_children() const override;

public:
  size_t _dominant_idx = 0;

protected:
  BinaryOpKind _op;
  Expr *_lhs = nullptr;
  Expr *_rhs = nullptr;
};

class MemberAccess : public BinaryOperator {
protected:
  explicit MemberAccess(SourceIndex loc);

public:
  static MemberAccess *Create(SourceIndex loc);

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
  UnaryOperator(UnaryOpKind op, SourceIndex loc);

public:
  static UnaryOperator *Create(UnaryOpKind op, SourceIndex loc);
  static UnaryOperator *Create(UnaryOpKind op, SourceIndex loc, Expr *rhs);

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
  BinaryOrUnary(SourceIndex loc, int bp);

public:
  static BinaryOrUnary *Create(SourceIndex loc, int bp);

  enum BinaryOrUnaryKind {
    UNKNOWN, BINARY, UNARY,
  };

  [[nodiscard]] BinaryOrUnaryKind get_kind() const;
  [[nodiscard]] BinaryOperator *get_bop() const;
  void set_bop(BinaryOperator *bop);
  [[nodiscard]] UnaryOperator *get_uop() const;
  void set_uop(UnaryOperator *uop);

  [[nodiscard]] Expr *get_generic_ptr() const;
  [[nodiscard]] ASTBase *get() const override;
  [[nodiscard]] ASTType *get_type() const override;
  void set_type(ASTType *type) override;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

private:
  BinaryOrUnaryKind _kind = UNKNOWN;
  union {
    BinaryOperator *_bop = nullptr;
    UnaryOperator *_uop;
  };
};

class Parenthesis : public Expr {
protected:
  explicit Parenthesis(SourceIndex loc);

public:
  static Parenthesis *Create(SourceIndex loc);

  void set_sub(Expr *sub);
  [[nodiscard]] Expr *get_sub() const;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

private:
  Expr *_sub = nullptr;
};

class FunctionCall : public Expr, public ASTNamed {
protected:
  explicit FunctionCall(SourceIndex loc);

public:
  static FunctionCall *Create(SourceIndex loc);
  [[nodiscard]] size_t get_n_args() const;
  [[nodiscard]] Expr *get_arg(size_t i) const;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

public:
  vector<Expr *> _args{};
  FunctionDecl *_callee = nullptr;
};

class Assignment : public Expr {
protected:
  explicit Assignment(SourceIndex loc);

public:
  static Assignment *Create(SourceIndex loc);

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
  explicit Cast(SourceIndex loc);

public:
  static Cast *Create(SourceIndex loc);
  [[nodiscard]] Expr *get_lhs() const;
  void set_lhs(Expr *lhs);
  [[nodiscard]] ASTBase *get_rhs() const;
  void set_rhs(ASTBase *rhs);

  [[nodiscard]] vector<ASTBase *> get_children() const override;

protected:
  Expr *_lhs = nullptr;
  ASTBase *_rhs = nullptr;
};

}

#endif //__TAN_SRC_AST_EXPR_H__
