#include "src/ast/expr.h"

using namespace tanlang;

/// \section Literals

IntegerLiteral *IntegerLiteral::Create(uint64_t val, bool is_unsigned) {
  auto ret = new IntegerLiteral;
  ret->_value = val;
  ret->_is_unsigned = is_unsigned;
  return ret;
}

FloatLiteral *FloatLiteral::Create(double val) {
  auto ret = new FloatLiteral;
  ret->_value = val;
  return ret;
}

double FloatLiteral::get_value() const { return _value; }

void FloatLiteral::set_value(double value) { _value = value; }

StringLiteral *StringLiteral::Create(const str &val) {
  auto ret = new StringLiteral;
  ret->_value = val;
  return ret;
}

str StringLiteral::get_value() const { return _value; }

StringLiteral::StringLiteral() : Literal(ASTNodeType::STRING_LITERAL, 0) {}

CharLiteral *CharLiteral::Create(uint8_t val) {
  auto ret = new CharLiteral;
  ret->_value = val;
  return ret;
}

void CharLiteral::set_value(uint8_t val) { _value = val; }

uint8_t CharLiteral::get_value() const { return _value; }

ArrayLiteral *ArrayLiteral::Create(vector<Literal *> val) {
  auto ret = new ArrayLiteral;
  ret->_elements = val;
  return ret;
}

void ArrayLiteral::set_elements(const vector<Literal *> &elements) { _elements = elements; }

ArrayLiteral *ArrayLiteral::Create() {
  return new ArrayLiteral;
}

vector<Literal *> ArrayLiteral::get_elements() const { return _elements; }

/// \section Identifier

Identifier::Identifier() : Expr(ASTNodeType::ID, 0) {}

Identifier *Identifier::Create(const str &name) {
  auto ret = new Identifier;
  ret->set_name(name);
  return ret;
}

/// \section Binary operators

BinaryOperator::BinaryOperator(BinaryOpKind op) : Expr(ASTNodeType::BOP, BinaryOperator::BOPPrecedence[op]), _op(op) {}

umap<BinaryOpKind, int>BinaryOperator::BOPPrecedence =
    {{BinaryOpKind::SUM, PREC_TERM}, {BinaryOpKind::SUBTRACT, PREC_TERM}, {BinaryOpKind::BOR, PREC_TERM},
        {BinaryOpKind::XOR, PREC_TERM}, {BinaryOpKind::MULTIPLY, PREC_FACTOR}, {BinaryOpKind::DIVIDE, PREC_FACTOR},
        {BinaryOpKind::MOD, PREC_FACTOR}, {BinaryOpKind::BAND, PREC_FACTOR}, {BinaryOpKind::GT, PREC_COMPARISON},
        {BinaryOpKind::GE, PREC_COMPARISON}, {BinaryOpKind::NE, PREC_COMPARISON}, {BinaryOpKind::LT, PREC_COMPARISON},
        {BinaryOpKind::LE, PREC_COMPARISON}, {BinaryOpKind::EQ, PREC_COMPARISON},
        {BinaryOpKind::LAND, PREC_LOGICAL_AND}, {BinaryOpKind::LOR, PREC_LOGICAL_OR},
        {BinaryOpKind::MEMBER_ACCESS, PREC_HIGHEST}};

BinaryOperator *BinaryOperator::Create(BinaryOpKind op) { return new BinaryOperator(op); }

BinaryOperator *BinaryOperator::Create(BinaryOpKind op, Expr *lhs, Expr *rhs) {
  auto ret = new BinaryOperator(op);
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
        {UnaryOpKind::PLUS, PREC_UNARY}, {UnaryOpKind::MINUS, PREC_UNARY}};

UnaryOperator::UnaryOperator(UnaryOpKind op) : Expr(ASTNodeType::UOP, UnaryOperator::UOPPrecedence[op]), _op(op) {}

void UnaryOperator::set_rhs(Expr *rhs) { _rhs = rhs; }

UnaryOperator *UnaryOperator::Create(UnaryOpKind op) { return new UnaryOperator(op); }

UnaryOperator *UnaryOperator::Create(UnaryOpKind op, Expr *rhs) {
  auto ret = new UnaryOperator(op);
  ret->_rhs = rhs;
  return ret;
}

UnaryOpKind UnaryOperator::get_op() const { return _op; }

Expr *UnaryOperator::get_rhs() const { return _rhs; }

/// \section Parenthesis

Parenthesis *Parenthesis::Create() { return new Parenthesis; }

Parenthesis::Parenthesis() : Expr(ASTNodeType::PARENTHESIS, ASTBase::OpPrecedence[ASTNodeType::PARENTHESIS]) {}

void Parenthesis::set_sub(Expr *sub) { _sub = sub; }

Expr *Parenthesis::get_sub() const { return _sub; }

/// \section MEMBER_ACCESS operator

MemberAccess *MemberAccess::Create() { return new MemberAccess; }

/// \section Function call

FunctionCall *FunctionCall::Create() { return new FunctionCall; }

FunctionCall::FunctionCall() : Expr(ASTNodeType::FUNC_CALL, PREC_LOWEST) {}

/// \section Assignment

Expr *Assignment::get_rhs() const { return _rhs; }

void Assignment::set_rhs(Expr *rhs) { _rhs = rhs; }

Assignment *Assignment::Create() { return new Assignment; }

Assignment::Assignment() : Expr(ASTNodeType::ASSIGN, ASTBase::OpPrecedence[ASTNodeType::ASSIGN]) {}

ASTBase *Assignment::get_lhs() const { return _lhs; }

void Assignment::set_lhs(ASTBase *lhs) { _lhs = lhs; }

/// \section Cast

Expr *Cast::get_lhs() const { return _lhs; }

void Cast::set_lhs(Expr *lhs) { _lhs = lhs; }

ASTType *Cast::get_dest_type() const { return _dest_type; }

void Cast::set_dest_type(ASTType *dest_type) { _dest_type = dest_type; }

Cast *Cast::Create() { return new Cast; }

Cast::Cast() : Expr(ASTNodeType::CAST, ASTBase::OpPrecedence[ASTNodeType::CAST]) {}

ASTBase *Cast::get_rhs() const { return _rhs; }

void Cast::set_rhs(ASTBase *rhs) { _rhs = rhs; }

BinaryOrUnary::BinaryOrUnary(int lbp) : Expr(ASTNodeType::BOP_OR_UOP, lbp) {}

BinaryOrUnary *BinaryOrUnary::Create(int lbp) { return new BinaryOrUnary(lbp); }

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
