#include "ast/expr.h"
#include "ast/type.h"
#include "ast/decl.h"
#include <utility>
#include <algorithm>

using namespace tanlang;

Expr::Expr(ASTNodeType type, SrcLoc loc, int bp) : ASTBase(type, loc, bp) {}

vector<ASTBase *> Expr::get_children() const { return {}; }

/// \section Literals

Literal::Literal(ASTNodeType type, SrcLoc loc, int bp) : CompTimeExpr(type, loc, bp) {}

BoolLiteral *BoolLiteral::Create(SrcLoc loc, bool val) {
  auto ret = new BoolLiteral(loc);
  ret->_value = val;
  return ret;
}

bool BoolLiteral::get_value() const { return _value; }

BoolLiteral::BoolLiteral(SrcLoc loc) : Literal(ASTNodeType::BOOL_LITERAL, loc, 0) {}

IntegerLiteral::IntegerLiteral(SrcLoc loc) : Literal(ASTNodeType::INTEGER_LITERAL, loc, 0) {}

IntegerLiteral *IntegerLiteral::Create(SrcLoc loc, uint64_t val, bool is_unsigned) {
  auto ret = new IntegerLiteral(loc);
  ret->_value = val;
  ret->_is_unsigned = is_unsigned;
  return ret;
}

FloatLiteral *FloatLiteral::Create(SrcLoc loc, double val) {
  auto ret = new FloatLiteral(loc);
  ret->_value = val;
  return ret;
}

double FloatLiteral::get_value() const { return _value; }

void FloatLiteral::set_value(double value) { _value = value; }

FloatLiteral::FloatLiteral(SrcLoc loc) : Literal(ASTNodeType::FLOAT_LITERAL, loc, 0) {}

StringLiteral *StringLiteral::Create(SrcLoc loc, const str &val) {
  auto ret = new StringLiteral(loc);
  ret->_value = val;
  return ret;
}

str StringLiteral::get_value() const { return _value; }

StringLiteral::StringLiteral(SrcLoc loc) : Literal(ASTNodeType::STRING_LITERAL, loc, 0) {}

CharLiteral *CharLiteral::Create(SrcLoc loc, uint8_t val) {
  auto ret = new CharLiteral(loc);
  ret->_value = val;
  return ret;
}

void CharLiteral::set_value(uint8_t val) { _value = val; }

uint8_t CharLiteral::get_value() const { return _value; }

CharLiteral::CharLiteral(SrcLoc loc) : Literal(ASTNodeType::CHAR_LITERAL, loc, 0) {}

ArrayLiteral *ArrayLiteral::Create(SrcLoc loc, vector<Literal *> val) {
  auto ret = new ArrayLiteral(loc);
  ret->_elements = std::move(val);
  return ret;
}

ArrayLiteral *ArrayLiteral::Create(SrcLoc loc) { return new ArrayLiteral(loc); }

void ArrayLiteral::set_elements(const vector<Literal *> &elements) { _elements = elements; }

vector<Literal *> ArrayLiteral::get_elements() const { return _elements; }

ArrayLiteral::ArrayLiteral(SrcLoc loc) : Literal(ASTNodeType::ARRAY_LITERAL, loc, 0) {}

NullPointerLiteral::NullPointerLiteral(SrcLoc loc) : Literal(ASTNodeType::NULLPTR_LITERAL, loc, 0) {}

NullPointerLiteral *NullPointerLiteral::Create(SrcLoc loc) { return new NullPointerLiteral(loc); }

/// \section Identifier

VarRef *VarRef::Create(SrcLoc loc, const str &name, Decl *referred) {
  auto ret = new VarRef(loc);
  ret->set_name(name);
  ret->_referred = referred;
  return ret;
}

VarRef::VarRef(SrcLoc loc) : Expr(ASTNodeType::VAR_REF, loc, 0) { _is_lvalue = true; }

Decl *VarRef::get_referred() const { return _referred; }

Type *VarRef::get_type() const {
  TAN_ASSERT(_referred);
  return _referred->get_type();
}

void VarRef::set_type(Type *) { TAN_ASSERT(false); }

Identifier::Identifier(SrcLoc loc) : Expr(ASTNodeType::ID, loc, 0) {}

Identifier *Identifier::Create(SrcLoc loc, const str &name) {
  auto ret = new Identifier(loc);
  ret->set_name(name);
  return ret;
}

IdentifierType Identifier::get_id_type() const { return _id_type; }

VarRef *Identifier::get_var_ref() const {
  TAN_ASSERT(_id_type == IdentifierType::ID_VAR_DECL);
  return _var_ref;
}

Type *Identifier::get_type_ref() const {
  TAN_ASSERT(_id_type == IdentifierType::ID_TYPE_DECL);
  return _type_ref;
}

void Identifier::set_var_ref(VarRef *var_ref) {
  _id_type = IdentifierType::ID_VAR_DECL;
  _var_ref = var_ref;
}

void Identifier::set_type_ref(Type *type_ref) {
  _id_type = IdentifierType::ID_TYPE_DECL;
  _type_ref = type_ref;
}

bool Identifier::is_lvalue() {
  if (_id_type == IdentifierType::ID_VAR_DECL) {
    return true;
  }
  return false;
}

void Identifier::set_lvalue(bool) { TAN_ASSERT(false); }

/// \section Binary operators

BinaryOperator::BinaryOperator(BinaryOpKind op, SrcLoc loc)
    : Expr(ASTNodeType::BOP, loc, BinaryOperator::BOPPrecedence[op]), _op(op) {}

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

BinaryOperator *BinaryOperator::Create(BinaryOpKind op, SrcLoc loc) { return new BinaryOperator(op, loc); }

BinaryOperator *BinaryOperator::Create(BinaryOpKind op, SrcLoc loc, Expr *lhs, Expr *rhs) {
  auto ret = new BinaryOperator(op, loc);
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

UnaryOperator::UnaryOperator(UnaryOpKind op, SrcLoc loc)
    : Expr(ASTNodeType::UOP, loc, UnaryOperator::UOPPrecedence[op]), _op(op) {}

void UnaryOperator::set_rhs(Expr *rhs) { _rhs = rhs; }

UnaryOperator *UnaryOperator::Create(UnaryOpKind op, SrcLoc loc) { return new UnaryOperator(op, loc); }

UnaryOperator *UnaryOperator::Create(UnaryOpKind op, SrcLoc loc, Expr *rhs) {
  auto ret = new UnaryOperator(op, loc);
  ret->_rhs = rhs;
  return ret;
}

UnaryOpKind UnaryOperator::get_op() const { return _op; }

Expr *UnaryOperator::get_rhs() const { return _rhs; }

vector<ASTBase *> UnaryOperator::get_children() const { return {_rhs}; }

/// \section Parenthesis

Parenthesis *Parenthesis::Create(SrcLoc loc) { return new Parenthesis(loc); }

Parenthesis::Parenthesis(SrcLoc loc)
    : Expr(ASTNodeType::PARENTHESIS, loc, ASTBase::OpPrecedence[ASTNodeType::PARENTHESIS]) {}

void Parenthesis::set_sub(Expr *sub) { _sub = sub; }

Expr *Parenthesis::get_sub() const { return _sub; }

vector<ASTBase *> Parenthesis::get_children() const { return {_sub}; }

void Parenthesis::set_lvalue(bool) { TAN_ASSERT(false); }

bool Parenthesis::is_lvalue() { return _sub->is_lvalue(); }

/// \section MEMBER_ACCESS operator

MemberAccess *MemberAccess::Create(SrcLoc loc) { return new MemberAccess(loc); }

MemberAccess::MemberAccess(SrcLoc loc) : BinaryOperator(BinaryOpKind::MEMBER_ACCESS, loc) {}

void MemberAccess::set_lvalue(bool) { TAN_ASSERT(false); }

bool MemberAccess::is_lvalue() {
  TAN_ASSERT(_lhs);
  return _lhs->is_lvalue();
}

/// \section Function call

FunctionCall *FunctionCall::Create(SrcLoc loc) { return new FunctionCall(loc); }

FunctionCall::FunctionCall(SrcLoc loc) : Expr(ASTNodeType::FUNC_CALL, loc, PREC_LOWEST) {}

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

Assignment *Assignment::Create(SrcLoc loc) { return new Assignment(loc); }

Assignment::Assignment(SrcLoc loc) : Expr(ASTNodeType::ASSIGN, loc, ASTBase::OpPrecedence[ASTNodeType::ASSIGN]) {}

ASTBase *Assignment::get_lhs() const { return _lhs; }

void Assignment::set_lhs(ASTBase *lhs) { _lhs = lhs; }

vector<ASTBase *> Assignment::get_children() const { return {_lhs, _rhs}; }

/// \section Cast

Expr *Cast::get_lhs() const { return _lhs; }

void Cast::set_lhs(Expr *lhs) { _lhs = lhs; }

bool Cast::is_lvalue() { return _lhs->is_lvalue(); }

void Cast::set_lvalue(bool) { TAN_ASSERT(false); }

Cast *Cast::Create(SrcLoc loc) { return new Cast(loc); }

Cast::Cast(SrcLoc loc) : Expr(ASTNodeType::CAST, loc, ASTBase::OpPrecedence[ASTNodeType::CAST]) {}

vector<ASTBase *> Cast::get_children() const { return {_lhs}; }

BinaryOrUnary::BinaryOrUnary(SrcLoc loc, int bp) : Expr(ASTNodeType::BOP_OR_UOP, loc, bp) {}

BinaryOrUnary *BinaryOrUnary::Create(SrcLoc loc, int bp) { return new BinaryOrUnary(loc, bp); }

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

CompTimeExpr::CompTimeExpr(ASTNodeType type, SrcLoc loc, int bp) : Expr(type, loc, bp) {}

IntegerLiteral *Literal::CreateIntegerLiteral(SrcLoc loc, uint64_t val, size_t bit_size, bool is_unsigned) {
  auto *ret = IntegerLiteral::Create(loc, val, is_unsigned);
  ret->set_type(Type::GetIntegerType(bit_size, is_unsigned));
  return ret;
}

BoolLiteral *Literal::CreateBoolLiteral(SrcLoc loc, bool val) {
  auto *ret = BoolLiteral::Create(loc, val);
  ret->set_type(Type::GetBoolType());
  return ret;
}

FloatLiteral *Literal::CreateFloatLiteral(SrcLoc loc, double val, size_t bit_size) {
  auto *ret = FloatLiteral::Create(loc, val);
  ret->set_type(Type::GetFloatType(bit_size));
  return ret;
}

StringLiteral *Literal::CreateStringLiteral(SrcLoc loc, str val) {
  auto *ret = StringLiteral::Create(loc, val);
  ret->set_type(Type::GetStringType());
  return ret;
}

CharLiteral *Literal::CreateCharLiteral(SrcLoc loc, uint8_t val) {
  auto *ret = CharLiteral::Create(loc, val);
  ret->set_type(Type::GetCharType());
  return ret;
}

ArrayLiteral *Literal::CreateArrayLiteral(SrcLoc loc, Type *element_type, int size) {
  auto *ret = ArrayLiteral::Create(loc);
  vector<Type *> sub_types{};
  ret->set_type(Type::GetArrayType(element_type, size));
  return ret;
}

NullPointerLiteral *Literal::CreateNullPointerLiteral(SrcLoc loc, Type *element_type) {
  auto *ret = NullPointerLiteral::Create(loc);
  ret->set_type(Type::GetPointerType(element_type));
  return ret;
}