#include "src/ast/expr.h"

using namespace tanlang;

Expr::Expr(ASTNodeType type, SourceIndex loc, int lbp) : ASTBase(type, loc, lbp) {}

/// \section Literals

Literal::Literal(ASTNodeType type, SourceIndex loc, int lbp) : CompTimeExpr(type, loc, lbp) {}

IntegerLiteral::IntegerLiteral(SourceIndex loc) : Literal(ASTNodeType::INTEGER_LITERAL, loc, 0) {}

IntegerLiteral *IntegerLiteral::Create(SourceIndex loc, uint64_t val, bool is_unsigned) {
  auto ret = new IntegerLiteral(loc);
  ret->_value = val;
  ret->_is_unsigned = is_unsigned;
  return ret;
}

FloatLiteral *FloatLiteral::Create(SourceIndex loc, double val) {
  auto ret = new FloatLiteral(loc);
  ret->_value = val;
  return ret;
}

double FloatLiteral::get_value() const { return _value; }

void FloatLiteral::set_value(double value) { _value = value; }

FloatLiteral::FloatLiteral(SourceIndex loc) : Literal(ASTNodeType::FLOAT_LITERAL, loc, 0) {}

StringLiteral *StringLiteral::Create(SourceIndex loc, const str &val) {
  auto ret = new StringLiteral(loc);
  ret->_value = val;
  return ret;
}

str StringLiteral::get_value() const { return _value; }

StringLiteral::StringLiteral(SourceIndex loc) : Literal(ASTNodeType::STRING_LITERAL, loc, 0) {}

CharLiteral *CharLiteral::Create(SourceIndex loc, uint8_t val) {
  auto ret = new CharLiteral(loc);
  ret->_value = val;
  return ret;
}

void CharLiteral::set_value(uint8_t val) { _value = val; }

uint8_t CharLiteral::get_value() const { return _value; }

CharLiteral::CharLiteral(SourceIndex loc) : Literal(ASTNodeType::CHAR_LITERAL, loc, 0) {}

ArrayLiteral *ArrayLiteral::Create(SourceIndex loc, vector<Literal *> val) {
  auto ret = new ArrayLiteral(loc);
  ret->_elements = val;
  return ret;
}

ArrayLiteral *ArrayLiteral::Create(SourceIndex loc) {
  return new ArrayLiteral(loc);
}

void ArrayLiteral::set_elements(const vector<Literal *> &elements) { _elements = elements; }

vector<Literal *> ArrayLiteral::get_elements() const { return _elements; }

ArrayLiteral::ArrayLiteral(SourceIndex loc) : Literal(ASTNodeType::ARRAY_LITERAL, loc, 0) {}

/// \section Identifier

Identifier::Identifier(SourceIndex loc) : Expr(ASTNodeType::ID, loc, 0) {}

Identifier *Identifier::Create(SourceIndex loc, const str &name) {
  auto ret = new Identifier(loc);
  ret->set_name(name);
  return ret;
}

/// \section Binary operators

BinaryOperator::BinaryOperator(BinaryOpKind op, SourceIndex loc) : Expr(ASTNodeType::BOP,
    loc,
    BinaryOperator::BOPPrecedence[op]), _op(op) {}

umap<BinaryOpKind, int>BinaryOperator::BOPPrecedence =
    {{BinaryOpKind::SUM, PREC_TERM}, {BinaryOpKind::SUBTRACT, PREC_TERM}, {BinaryOpKind::BOR, PREC_TERM},
        {BinaryOpKind::XOR, PREC_TERM}, {BinaryOpKind::MULTIPLY, PREC_FACTOR}, {BinaryOpKind::DIVIDE, PREC_FACTOR},
        {BinaryOpKind::MOD, PREC_FACTOR}, {BinaryOpKind::BAND, PREC_FACTOR}, {BinaryOpKind::GT, PREC_COMPARISON},
        {BinaryOpKind::GE, PREC_COMPARISON}, {BinaryOpKind::NE, PREC_COMPARISON}, {BinaryOpKind::LT, PREC_COMPARISON},
        {BinaryOpKind::LE, PREC_COMPARISON}, {BinaryOpKind::EQ, PREC_COMPARISON},
        {BinaryOpKind::LAND, PREC_LOGICAL_AND}, {BinaryOpKind::LOR, PREC_LOGICAL_OR},
        {BinaryOpKind::MEMBER_ACCESS, PREC_HIGHEST}};

BinaryOperator *BinaryOperator::Create(BinaryOpKind op, SourceIndex loc) { return new BinaryOperator(op, loc); }

BinaryOperator *BinaryOperator::Create(BinaryOpKind op, SourceIndex loc, Expr *lhs, Expr *rhs) {
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

/// \section Unary operators

umap<UnaryOpKind, int>UnaryOperator::UOPPrecedence =
    {{UnaryOpKind::BNOT, PREC_UNARY}, {UnaryOpKind::LNOT, PREC_UNARY}, {UnaryOpKind::ADDRESS_OF, PREC_UNARY},
        {UnaryOpKind::PLUS, PREC_UNARY}, {UnaryOpKind::MINUS, PREC_UNARY}, {UnaryOpKind::PTR_DEREF, PREC_HIGHEST}};

UnaryOperator::UnaryOperator(UnaryOpKind op, SourceIndex loc) : Expr(ASTNodeType::UOP,
    loc,
    UnaryOperator::UOPPrecedence[op]), _op(op) {}

void UnaryOperator::set_rhs(Expr *rhs) { _rhs = rhs; }

UnaryOperator *UnaryOperator::Create(UnaryOpKind op, SourceIndex loc) { return new UnaryOperator(op, loc); }

UnaryOperator *UnaryOperator::Create(UnaryOpKind op, SourceIndex loc, Expr *rhs) {
  auto ret = new UnaryOperator(op, loc);
  ret->_rhs = rhs;
  return ret;
}

UnaryOpKind UnaryOperator::get_op() const { return _op; }

Expr *UnaryOperator::get_rhs() const { return _rhs; }

/// \section Parenthesis

Parenthesis *Parenthesis::Create(SourceIndex loc) { return new Parenthesis(loc); }

Parenthesis::Parenthesis(SourceIndex loc) : Expr(ASTNodeType::PARENTHESIS,
    loc,
    ASTBase::OpPrecedence[ASTNodeType::PARENTHESIS]) {}

void Parenthesis::set_sub(Expr *sub) { _sub = sub; }

Expr *Parenthesis::get_sub() const { return _sub; }

/// \section MEMBER_ACCESS operator

MemberAccess *MemberAccess::Create(SourceIndex loc) { return new MemberAccess(loc); }

MemberAccess::MemberAccess(SourceIndex loc) : BinaryOperator(BinaryOpKind::MEMBER_ACCESS, loc) {}

/// \section Function call

FunctionCall *FunctionCall::Create(SourceIndex loc) { return new FunctionCall(loc); }

FunctionCall::FunctionCall(SourceIndex loc) : Expr(ASTNodeType::FUNC_CALL, loc, PREC_LOWEST) {}

/// \section Assignment

Expr *Assignment::get_rhs() const { return _rhs; }

void Assignment::set_rhs(Expr *rhs) { _rhs = rhs; }

Assignment *Assignment::Create(SourceIndex loc) { return new Assignment(loc); }

Assignment::Assignment(SourceIndex loc) : Expr(ASTNodeType::ASSIGN, loc, ASTBase::OpPrecedence[ASTNodeType::ASSIGN]) {}

ASTBase *Assignment::get_lhs() const { return _lhs; }

void Assignment::set_lhs(ASTBase *lhs) { _lhs = lhs; }

/// \section Cast

Expr *Cast::get_lhs() const { return _lhs; }

void Cast::set_lhs(Expr *lhs) { _lhs = lhs; }

Cast *Cast::Create(SourceIndex loc) { return new Cast(loc); }

Cast::Cast(SourceIndex loc) : Expr(ASTNodeType::CAST, loc, ASTBase::OpPrecedence[ASTNodeType::CAST]) {}

ASTBase *Cast::get_rhs() const { return _rhs; }

void Cast::set_rhs(ASTBase *rhs) { _rhs = rhs; }

BinaryOrUnary::BinaryOrUnary(SourceIndex loc, int lbp) : Expr(ASTNodeType::BOP_OR_UOP, loc, lbp) {}

BinaryOrUnary *BinaryOrUnary::Create(SourceIndex loc, int lbp) { return new BinaryOrUnary(loc, lbp); }

BinaryOrUnary::BinaryOrUnaryKind BinaryOrUnary::get_kind() const { return _kind; }

BinaryOperator *BinaryOrUnary::get_bop() const {
  TAN_ASSERT(_kind == BINARY);
  return _bop;
}

void BinaryOrUnary::set_bop(BinaryOperator *bop) {
  TAN_ASSERT(_kind == UNKNOWN); /// prevent setting this twice
  _kind = BINARY;
  _bop = bop;
}

UnaryOperator *BinaryOrUnary::get_uop() const {
  TAN_ASSERT(_kind == UNARY);
  return _uop;
}

void BinaryOrUnary::set_uop(UnaryOperator *uop) {
  TAN_ASSERT(_kind == UNKNOWN); /// prevent setting this twice
  _kind = UNARY;
  _uop = uop;
}

Expr *BinaryOrUnary::get_generic_ptr() const {
  switch (_kind) {
    case BINARY:
      return _bop;
    case UNARY:
      return _uop;
    default:
      TAN_ASSERT(false);
      break;
  }
}

ASTBase *BinaryOrUnary::get() const { return get_generic_ptr(); }

ASTType *BinaryOrUnary::get_type() const { return get_generic_ptr()->get_type(); }

void BinaryOrUnary::set_type(ASTType *type) { get_generic_ptr()->set_type(type); }

bool CompTimeExpr::is_comptime_known() { return true; }

CompTimeExpr::CompTimeExpr(ASTNodeType type, SourceIndex loc, int lbp) : Expr(type, loc, lbp) {}
