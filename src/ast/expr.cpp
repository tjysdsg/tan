#include "ast/expr.h"
#include "ast/type.h"
#include "ast/decl.h"
#include <utility>
#include <algorithm>

using namespace tanlang;

Expr::Expr(ASTNodeType type, SourceFile *src, int bp) : ASTBase(type, src, bp) {}

vector<ASTBase *> Expr::get_children() const { return {}; }

/// \section Literals

Literal::Literal(ASTNodeType type, SourceFile *src, int bp) : CompTimeExpr(type, src, bp) {}

BoolLiteral *BoolLiteral::Create(SourceFile *src, bool val) {
  auto ret = new BoolLiteral(src);
  ret->_value = val;
  return ret;
}

bool BoolLiteral::get_value() const { return _value; }

BoolLiteral::BoolLiteral(SourceFile *src) : Literal(ASTNodeType::BOOL_LITERAL, src, 0) {}

IntegerLiteral::IntegerLiteral(SourceFile *src) : Literal(ASTNodeType::INTEGER_LITERAL, src, 0) {}

IntegerLiteral *IntegerLiteral::Create(SourceFile *src, uint64_t val, bool is_unsigned) {
  auto ret = new IntegerLiteral(src);
  ret->_value = val;
  ret->_is_unsigned = is_unsigned;
  return ret;
}

FloatLiteral *FloatLiteral::Create(SourceFile *src, double val) {
  auto ret = new FloatLiteral(src);
  ret->_value = val;
  return ret;
}

double FloatLiteral::get_value() const { return _value; }

void FloatLiteral::set_value(double value) { _value = value; }

FloatLiteral::FloatLiteral(SourceFile *src) : Literal(ASTNodeType::FLOAT_LITERAL, src, 0) {}

StringLiteral *StringLiteral::Create(SourceFile *src, const str &val) {
  auto ret = new StringLiteral(src);
  ret->_value = val;
  return ret;
}

str StringLiteral::get_value() const { return _value; }

StringLiteral::StringLiteral(SourceFile *src) : Literal(ASTNodeType::STRING_LITERAL, src, 0) {}

CharLiteral *CharLiteral::Create(SourceFile *src, uint8_t val) {
  auto ret = new CharLiteral(src);
  ret->_value = val;
  return ret;
}

void CharLiteral::set_value(uint8_t val) { _value = val; }

uint8_t CharLiteral::get_value() const { return _value; }

CharLiteral::CharLiteral(SourceFile *src) : Literal(ASTNodeType::CHAR_LITERAL, src, 0) {}

ArrayLiteral *ArrayLiteral::Create(SourceFile *src, vector<Literal *> val) {
  auto ret = new ArrayLiteral(src);
  ret->_elements = std::move(val);
  return ret;
}

ArrayLiteral *ArrayLiteral::Create(SourceFile *src) { return new ArrayLiteral(src); }

void ArrayLiteral::set_elements(const vector<Literal *> &elements) { _elements = elements; }

vector<Literal *> ArrayLiteral::get_elements() const { return _elements; }

ArrayLiteral::ArrayLiteral(SourceFile *src) : Literal(ASTNodeType::ARRAY_LITERAL, src, 0) {}

NullPointerLiteral::NullPointerLiteral(SourceFile *src) : Literal(ASTNodeType::NULLPTR_LITERAL, src, 0) {}

NullPointerLiteral *NullPointerLiteral::Create(SourceFile *src) { return new NullPointerLiteral(src); }

/// \section Identifier

VarRef *VarRef::Create(SourceFile *src, const str &name, Decl *referred) {
  auto ret = new VarRef(src);
  ret->set_name(name);
  ret->_referred = referred;
  return ret;
}

VarRef::VarRef(SourceFile *src) : Expr(ASTNodeType::VAR_REF, src, 0) { _is_lvalue = true; }

Decl *VarRef::get_referred() const { return _referred; }

Type *VarRef::get_type() const {
  TAN_ASSERT(_referred);
  return _referred->get_type();
}

void VarRef::set_type(Type *) { TAN_ASSERT(false); }

Identifier::Identifier(SourceFile *src) : Expr(ASTNodeType::ID, src, 0) {}

Identifier *Identifier::Create(SourceFile *src, const str &name) {
  auto ret = new Identifier(src);
  ret->set_name(name);
  return ret;
}

IdentifierType Identifier::get_id_type() const { return _id_type; }

VarRef *Identifier::get_var_ref() const {
  TAN_ASSERT(_id_type == IdentifierType::ID_VAR_REF);
  return _var_ref;
}

void Identifier::set_var_ref(VarRef *var_ref) {
  _id_type = IdentifierType::ID_VAR_REF;
  _var_ref = var_ref;
}

void Identifier::set_type_ref(Type *type_ref) {
  _id_type = IdentifierType::ID_TYPE_REF;
  set_type(type_ref);
}

bool Identifier::is_lvalue() {
  TAN_ASSERT(_id_type != IdentifierType::INVALID);
  if (_id_type == IdentifierType::ID_VAR_REF) {
    return true;
  }
  return false;
}

void Identifier::set_lvalue(bool) { TAN_ASSERT(false); }

/// \section Binary operators

BinaryOperator::BinaryOperator(BinaryOpKind op, SourceFile *src)
    : Expr(ASTNodeType::BOP, src, BinaryOperator::BOPPrecedence[op]), _op(op) {}

umap<BinaryOpKind, int> BinaryOperator::BOPPrecedence = {
    {BinaryOpKind::SUM,           PREC_TERM       },
    {BinaryOpKind::SUBTRACT,      PREC_TERM       },
    {BinaryOpKind::BOR,           PREC_TERM       },
    {BinaryOpKind::XOR,           PREC_TERM       },
    {BinaryOpKind::MULTIPLY,      PREC_FACTOR     },
    {BinaryOpKind::DIVIDE,        PREC_FACTOR     },
    {BinaryOpKind::MOD,           PREC_FACTOR     },
    {BinaryOpKind::BAND,          PREC_FACTOR     },
    {BinaryOpKind::GT,            PREC_COMPARISON },
    {BinaryOpKind::GE,            PREC_COMPARISON },
    {BinaryOpKind::NE,            PREC_COMPARISON },
    {BinaryOpKind::LT,            PREC_COMPARISON },
    {BinaryOpKind::LE,            PREC_COMPARISON },
    {BinaryOpKind::EQ,            PREC_COMPARISON },
    {BinaryOpKind::LAND,          PREC_LOGICAL_AND},
    {BinaryOpKind::LOR,           PREC_LOGICAL_OR },
    {BinaryOpKind::MEMBER_ACCESS, PREC_HIGHEST    }
};

BinaryOperator *BinaryOperator::Create(BinaryOpKind op, SourceFile *src) { return new BinaryOperator(op, src); }

BinaryOperator *BinaryOperator::Create(BinaryOpKind op, SourceFile *src, Expr *lhs, Expr *rhs) {
  auto ret = new BinaryOperator(op, src);
  ret->_lhs = lhs;
  ret->_rhs = rhs;
  return ret;
}

void BinaryOperator::set_lhs(Expr *lhs) { _lhs = lhs; }

void BinaryOperator::set_rhs(Expr *rhs) { _rhs = rhs; }

Expr *BinaryOperator::get_lhs() const { return _lhs; }

Expr *BinaryOperator::get_rhs() const { return _rhs; }

BinaryOpKind BinaryOperator::get_op() const { return _op; }

vector<ASTBase *> BinaryOperator::get_children() const { return {_lhs, _rhs}; }

/// \section Unary operators

umap<UnaryOpKind, int> UnaryOperator::UOPPrecedence = {
    {UnaryOpKind::BNOT,       PREC_UNARY  },
    {UnaryOpKind::LNOT,       PREC_UNARY  },
    {UnaryOpKind::ADDRESS_OF, PREC_UNARY  },
    {UnaryOpKind::PLUS,       PREC_UNARY  },
    {UnaryOpKind::MINUS,      PREC_UNARY  },
    {UnaryOpKind::PTR_DEREF,  PREC_HIGHEST}
};

UnaryOperator::UnaryOperator(UnaryOpKind op, SourceFile *src)
    : Expr(ASTNodeType::UOP, src, UnaryOperator::UOPPrecedence[op]), _op(op) {}

void UnaryOperator::set_rhs(Expr *rhs) { _rhs = rhs; }

UnaryOperator *UnaryOperator::Create(UnaryOpKind op, SourceFile *src) { return new UnaryOperator(op, src); }

UnaryOperator *UnaryOperator::Create(UnaryOpKind op, SourceFile *src, Expr *rhs) {
  auto ret = new UnaryOperator(op, src);
  ret->_rhs = rhs;
  return ret;
}

UnaryOpKind UnaryOperator::get_op() const { return _op; }

Expr *UnaryOperator::get_rhs() const { return _rhs; }

vector<ASTBase *> UnaryOperator::get_children() const { return {_rhs}; }

/// \section Parenthesis

Parenthesis *Parenthesis::Create(SourceFile *src) { return new Parenthesis(src); }

Parenthesis::Parenthesis(SourceFile *src)
    : Expr(ASTNodeType::PARENTHESIS, src, ASTBase::OpPrecedence[ASTNodeType::PARENTHESIS]) {}

void Parenthesis::set_sub(Expr *sub) { _sub = sub; }

Expr *Parenthesis::get_sub() const { return _sub; }

vector<ASTBase *> Parenthesis::get_children() const { return {_sub}; }

void Parenthesis::set_lvalue(bool) { TAN_ASSERT(false); }

bool Parenthesis::is_lvalue() { return _sub->is_lvalue(); }

/// \section MEMBER_ACCESS operator

MemberAccess *MemberAccess::Create(SourceFile *src) { return new MemberAccess(src); }

MemberAccess::MemberAccess(SourceFile *src) : BinaryOperator(BinaryOpKind::MEMBER_ACCESS, src) {}

void MemberAccess::set_lvalue(bool) { TAN_ASSERT(false); }

bool MemberAccess::is_lvalue() {
  TAN_ASSERT(_lhs);
  return _lhs->is_lvalue();
}

/// \section Function call

FunctionCall *FunctionCall::Create(SourceFile *src) { return new FunctionCall(src); }

FunctionCall::FunctionCall(SourceFile *src) : Expr(ASTNodeType::FUNC_CALL, src, PREC_LOWEST) {}

size_t FunctionCall::get_n_args() const { return _args.size(); }

Expr *FunctionCall::get_arg(size_t i) const {
  TAN_ASSERT(i < _args.size());
  return _args[i];
}

vector<ASTBase *> FunctionCall::get_children() const {
  vector<ASTBase *> ret = {(ASTBase *)_callee};
  std::for_each(_args.begin(), _args.end(), [&](Expr *e) { ret.push_back(e); });
  return ret;
}

/// \section Assignment

Expr *Assignment::get_rhs() const { return _rhs; }

void Assignment::set_rhs(Expr *rhs) { _rhs = rhs; }

Assignment *Assignment::Create(SourceFile *src) { return new Assignment(src); }

Assignment::Assignment(SourceFile *src) : Expr(ASTNodeType::ASSIGN, src, ASTBase::OpPrecedence[ASTNodeType::ASSIGN]) {}

ASTBase *Assignment::get_lhs() const { return _lhs; }

void Assignment::set_lhs(ASTBase *lhs) { _lhs = lhs; }

vector<ASTBase *> Assignment::get_children() const { return {_lhs, _rhs}; }

/// \section Cast

Expr *Cast::get_lhs() const { return _lhs; }

void Cast::set_lhs(Expr *lhs) { _lhs = lhs; }

bool Cast::is_lvalue() { return _lhs->is_lvalue(); }

void Cast::set_lvalue(bool) { TAN_ASSERT(false); }

Cast *Cast::Create(SourceFile *src) { return new Cast(src); }

Cast::Cast(SourceFile *src) : Expr(ASTNodeType::CAST, src, ASTBase::OpPrecedence[ASTNodeType::CAST]) {}

vector<ASTBase *> Cast::get_children() const { return {_lhs}; }

BinaryOrUnary::BinaryOrUnary(SourceFile *src, int bp) : Expr(ASTNodeType::BOP_OR_UOP, src, bp) {}

BinaryOrUnary *BinaryOrUnary::Create(SourceFile *src, int bp) { return new BinaryOrUnary(src, bp); }

void BinaryOrUnary::set_bop(BinaryOperator *bop) {
  TAN_ASSERT(_kind == UNKNOWN); /// prevent setting this twice
  _kind = BINARY;
  _bop = bop;
}

void BinaryOrUnary::set_uop(UnaryOperator *uop) {
  TAN_ASSERT(_kind == UNKNOWN); /// prevent setting this twice
  _kind = UNARY;
  _uop = uop;
}

Expr *BinaryOrUnary::get_expr_ptr() const {
  switch (_kind) {
  case BINARY:
    return _bop;
  case UNARY:
    return _uop;
  default:
    TAN_ASSERT(false);
  }
}

ASTBase *BinaryOrUnary::get() const { return get_expr_ptr(); }

Type *BinaryOrUnary::get_type() const { return get_expr_ptr()->get_type(); }

void BinaryOrUnary::set_type(Type *type) { get_expr_ptr()->set_type(type); }

vector<ASTBase *> BinaryOrUnary::get_children() const { return get_expr_ptr()->get_children(); }

bool BinaryOrUnary::is_lvalue() { return get_expr_ptr()->is_lvalue(); }

void BinaryOrUnary::set_lvalue(bool is_lvalue) { get_expr_ptr()->set_lvalue(is_lvalue); }

bool CompTimeExpr::is_comptime_known() { return true; }

CompTimeExpr::CompTimeExpr(ASTNodeType type, SourceFile *src, int bp) : Expr(type, src, bp) {}

IntegerLiteral *Literal::CreateIntegerLiteral(SourceFile *src, uint64_t val, size_t bit_size, bool is_unsigned) {
  auto *ret = IntegerLiteral::Create(src, val, is_unsigned);
  ret->set_type(Type::GetIntegerType(bit_size, is_unsigned));
  return ret;
}

BoolLiteral *Literal::CreateBoolLiteral(SourceFile *src, bool val) {
  auto *ret = BoolLiteral::Create(src, val);
  ret->set_type(Type::GetBoolType());
  return ret;
}

FloatLiteral *Literal::CreateFloatLiteral(SourceFile *src, double val, size_t bit_size) {
  auto *ret = FloatLiteral::Create(src, val);
  ret->set_type(Type::GetFloatType(bit_size));
  return ret;
}

StringLiteral *Literal::CreateStringLiteral(SourceFile *src, str val) {
  auto *ret = StringLiteral::Create(src, val);
  ret->set_type(Type::GetStringType());
  return ret;
}

CharLiteral *Literal::CreateCharLiteral(SourceFile *src, uint8_t val) {
  auto *ret = CharLiteral::Create(src, val);
  ret->set_type(Type::GetCharType());
  return ret;
}

ArrayLiteral *Literal::CreateArrayLiteral(SourceFile *src, Type *element_type, vector<Literal *> elements) {
  auto *ret = ArrayLiteral::Create(src, elements);

  vector<Type *> sub_types{};
  ret->set_type(Type::GetArrayType(element_type, (int)elements.size()));

  return ret;
}

NullPointerLiteral *Literal::CreateNullPointerLiteral(SourceFile *src, Type *element_type) {
  auto *ret = NullPointerLiteral::Create(src);
  ret->set_type(Type::GetPointerType(element_type));
  return ret;
}