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
