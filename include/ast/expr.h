#ifndef __TAN_SRC_AST_EXPR_H__
#define __TAN_SRC_AST_EXPR_H__
#include <utility>
#include "base.h"
#include "ast_base.h"
#include "ast_named.h"
#include "typed.h"
#include "fwd.h"

namespace tanlang {

class Expr : public ASTBase, public Typed {
protected:
  Expr(ASTNodeType type, TokenizedSourceFile *src, int bp);

public:
  virtual bool is_comptime_known() { return false; }
  virtual bool is_lvalue() { return _is_lvalue; }
  virtual void set_lvalue(bool is_lvalue) { _is_lvalue = is_lvalue; }
  [[nodiscard]] vector<ASTBase *> get_children() const override;

  bool is_expr() const override { return true; }

protected:
  bool _is_lvalue = false;
};

class Literal : public Expr {
protected:
  Literal(ASTNodeType type, TokenizedSourceFile *src, int bp);

public:
  static IntegerLiteral *CreateIntegerLiteral(TokenizedSourceFile *src, uint64_t val, size_t bit_size,
                                              bool is_unsigned);
  static BoolLiteral *CreateBoolLiteral(TokenizedSourceFile *src, bool val);
  static FloatLiteral *CreateFloatLiteral(TokenizedSourceFile *src, double val, size_t bit_size);
  static StringLiteral *CreateStringLiteral(TokenizedSourceFile *src, str val);
  static CharLiteral *CreateCharLiteral(TokenizedSourceFile *src, uint8_t val);
  static ArrayLiteral *CreateArrayLiteral(TokenizedSourceFile *src, Type *element_type,
                                          vector<Literal *> elements = {});
  static NullPointerLiteral *CreateNullPointerLiteral(TokenizedSourceFile *src, Type *element_type);

public:
  bool is_comptime_known() override { return true; }
};

class BoolLiteral : public Literal {
protected:
  explicit BoolLiteral(TokenizedSourceFile *src);

public:
  static BoolLiteral *Create(TokenizedSourceFile *src, bool val);
  [[nodiscard]] bool get_value() const;

private:
  bool _value = false;
};

class IntegerLiteral : public Literal {
protected:
  explicit IntegerLiteral(TokenizedSourceFile *src);

public:
  static IntegerLiteral *Create(TokenizedSourceFile *src, uint64_t val, bool is_unsigned = false);

  [[nodiscard]] uint64_t get_value() const { return _value; }
  [[nodiscard]] bool is_unsigned() const { return _is_unsigned; }

private:
  uint64_t _value = 0;
  bool _is_unsigned = false;
};

class FloatLiteral : public Literal {
protected:
  explicit FloatLiteral(TokenizedSourceFile *src);

public:
  static FloatLiteral *Create(TokenizedSourceFile *src, double val);
  [[nodiscard]] double get_value() const;
  void set_value(double value);

private:
  double _value = 0;
};

class StringLiteral : public Literal {
protected:
  explicit StringLiteral(TokenizedSourceFile *src);

public:
  static StringLiteral *Create(TokenizedSourceFile *src, const str &val);

  [[nodiscard]] str get_value() const;
  void set_value(str val) { _value = std::move(val); }

private:
  str _value;
};

class CharLiteral : public Literal {
protected:
  explicit CharLiteral(TokenizedSourceFile *src);

public:
  static CharLiteral *Create(TokenizedSourceFile *src, uint8_t val);

  void set_value(uint8_t val);
  [[nodiscard]] uint8_t get_value() const;

private:
  uint8_t _value = 0;
};

class ArrayLiteral : public Literal {
protected:
  explicit ArrayLiteral(TokenizedSourceFile *src);

public:
  static ArrayLiteral *Create(TokenizedSourceFile *src, vector<Literal *> val);
  static ArrayLiteral *Create(TokenizedSourceFile *src);

  void set_elements(const vector<Literal *> &elements);
  [[nodiscard]] vector<Literal *> get_elements() const;

private:
  vector<Literal *> _elements{};
};

class NullPointerLiteral : public Literal {
protected:
  explicit NullPointerLiteral(TokenizedSourceFile *src);

public:
  static NullPointerLiteral *Create(TokenizedSourceFile *src);
};

class VarRef : public Expr, public ASTNamed {
protected:
  explicit VarRef(TokenizedSourceFile *src);

public:
  static VarRef *Create(TokenizedSourceFile *src, const str &name, Decl *referred);
  [[nodiscard]] Decl *get_referred() const;

  [[nodiscard]] Type *get_type() const override;
  void set_type(Type *) override;

private:
  Decl *_referred = nullptr;
};

enum class IdentifierType {
  INVALID,
  ID_VAR_REF,
  ID_TYPE_REF,
};

class Identifier : public Expr, public ASTNamed {
protected:
  explicit Identifier(TokenizedSourceFile *src);

public:
  static Identifier *Create(TokenizedSourceFile *src, const str &name);

  [[nodiscard]] IdentifierType get_id_type() const;
  void set_var_ref(VarRef *var_ref);
  void set_type_ref(Type *type_ref);
  [[nodiscard]] VarRef *get_var_ref() const;
  bool is_lvalue() override;
  void set_lvalue(bool is_lvalue) override;

private:
  IdentifierType _id_type = IdentifierType::INVALID;
  VarRef *_var_ref = nullptr;
};

/// make sure to sync this with BinaryOperator::BOPPrecedence
enum class BinaryOpKind {
  SUM,           /// +
  SUBTRACT,      /// -
  MULTIPLY,      /// *
  DIVIDE,        /// /
  MOD,           /// %
  BAND,          /// binary and
  LAND,          /// logical and
  BOR,           /// binary or
  LOR,           /// logical or
  GT,            /// >
  GE,            /// >=
  LT,            /// <
  LE,            /// <=
  EQ,            /// ==
  NE,            /// !=
  XOR,           /// ^
  MEMBER_ACCESS, /// . or []
};

class BinaryOperator : public Expr {
protected:
  BinaryOperator(BinaryOpKind op, TokenizedSourceFile *src);

public:
  static BinaryOperator *Create(BinaryOpKind op, TokenizedSourceFile *src);
  static BinaryOperator *Create(BinaryOpKind op, TokenizedSourceFile *src, Expr *lhs, Expr *rhs);

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
  explicit MemberAccess(TokenizedSourceFile *src);

public:
  static MemberAccess *Create(TokenizedSourceFile *src);
  bool is_lvalue() override;
  void set_lvalue(bool is_lvalue) override;

public:
  enum {
    MemberAccessInvalid = 0,
    MemberAccessBracket,
    MemberAccessMemberVariable,
    MemberAccessMemberFunction,
  } _access_type = MemberAccessInvalid;
  int _access_idx = -1; /// struct member variable index
};

/// make sure to sync this with UnaryOperator::UOPPrecedence
enum class UnaryOpKind {
  INVALID,    ///
  BNOT,       /// bitwise not
  LNOT,       /// logical not
  PLUS,       /// +
  MINUS,      /// -
  ADDRESS_OF, /// var v: int; &v
  PTR_DEREF,  /// poitner dereference
};

class UnaryOperator : public Expr {
protected:
  UnaryOperator(UnaryOpKind op, TokenizedSourceFile *src);

public:
  static UnaryOperator *Create(UnaryOpKind op, TokenizedSourceFile *src);
  static UnaryOperator *Create(UnaryOpKind op, TokenizedSourceFile *src, Expr *rhs);

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
  BinaryOrUnary(TokenizedSourceFile *src, int bp);

public:
  static BinaryOrUnary *Create(TokenizedSourceFile *src, int bp);

  enum BinaryOrUnaryKind {
    UNKNOWN,
    BINARY,
    UNARY,
  };

  void set_bop(BinaryOperator *bop);
  void set_uop(UnaryOperator *uop);

  [[nodiscard]] Expr *get_expr_ptr() const;
  [[nodiscard]] ASTBase *get() const override;
  [[nodiscard]] Type *get_type() const override;
  void set_type(Type *type) override;

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
  explicit Parenthesis(TokenizedSourceFile *src);

public:
  static Parenthesis *Create(TokenizedSourceFile *src);

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
  explicit FunctionCall(TokenizedSourceFile *src);

public:
  static FunctionCall *Create(TokenizedSourceFile *src);
  [[nodiscard]] size_t get_n_args() const;
  [[nodiscard]] Expr *get_arg(size_t i) const;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

public:
  vector<Expr *> _args{};
  FunctionDecl *_callee = nullptr;
};

class Assignment : public Expr {
protected:
  explicit Assignment(TokenizedSourceFile *src);

public:
  static Assignment *Create(TokenizedSourceFile *src);

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
  explicit Cast(TokenizedSourceFile *src);

public:
  static Cast *Create(TokenizedSourceFile *src);
  [[nodiscard]] Expr *get_lhs() const;
  void set_lhs(Expr *lhs);

  [[nodiscard]] vector<ASTBase *> get_children() const override;

  bool is_lvalue() override;
  void set_lvalue(bool is_lvalue) override;
  bool is_comptime_known() override;

protected:
  Expr *_lhs = nullptr;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_EXPR_H__