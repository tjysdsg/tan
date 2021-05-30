#include "src/ast/expr.h"

using namespace tanlang;

/// \section Literals

ptr<IntegerLiteral> IntegerLiteral::Create(uint64_t val, bool is_unsigned) {
  auto ret = make_ptr<IntegerLiteral>();
  ret->_value = val;
  ret->_is_unsigned = is_unsigned;
  return ret;
}

ptr<FloatLiteral> FloatLiteral::Create(double val) {
  auto ret = make_ptr<FloatLiteral>();
  ret->_value = val;
  return ret;
}

ptr<StringLiteral> StringLiteral::Create(str_view val) {
  auto ret = make_ptr<StringLiteral>();
  ret->_value = val;
  return ret;
}

ptr<CharLiteral> CharLiteral::Create(uint8_t val) {
  auto ret = make_ptr<CharLiteral>();
  ret->_value = val;
  return ret;
}

ptr<ArrayLiteral> ArrayLiteral::Create(vector<ptr<Literal>> val) {
  auto ret = make_ptr<ArrayLiteral>();
  ret->_elements = val;
  return ret;
}

void ArrayLiteral::set_elements(const vector<ptr<Literal>> &elements) {
  _elements = elements;
}

ptr<ArrayLiteral> ArrayLiteral::Create() {
  return make_ptr<ArrayLiteral>();
}

/// \section Identifier

Identifier::Identifier() : Expr(ASTNodeType::ID, 0) {}

ptr<Identifier> Identifier::Create(str_view name) {
  auto ret = make_ptr<Identifier>();
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
        {BinaryOpKind::LE, PREC_COMPARISON}, {BinaryOpKind::EQ, PREC_COMPARISON}, {BinaryOpKind::ASSIGN, PREC_ASSIGN},
        {BinaryOpKind::BNOT, PREC_UNARY}, {BinaryOpKind::LNOT, PREC_UNARY}, {BinaryOpKind::LAND, PREC_LOGICAL_AND},
        {BinaryOpKind::LOR, PREC_LOGICAL_OR}};

ptr<BinaryOperator> BinaryOperator::Create(BinaryOpKind op) {
  return make_ptr<BinaryOperator>(op);
}

ptr<BinaryOperator> BinaryOperator::Create(BinaryOpKind op, const ptr<Expr> &lhs, const ptr<Expr> &rhs) {
  auto ret = make_ptr<BinaryOperator>(op);
  ret->_lhs = lhs;
  ret->_rhs = rhs;
  return ret;
}
