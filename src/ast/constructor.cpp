#include "src/ast/constructor.h"
#include "src/ast/expr.h"

using namespace tanlang;

BasicConstructor *BasicConstructor::Create(CompTimeExpr *val) {
  auto *ret = new BasicConstructor();
  ret->_value = val;
  return ret;
}

BasicConstructor *BasicConstructor::CreateIntegerConstructor(uint64_t default_val, bool is_unsigned) {
  return BasicConstructor::Create(IntegerLiteral::Create(default_val, is_unsigned));
}

BasicConstructor *BasicConstructor::CreateFPConstructor(double default_val) {
  return BasicConstructor::Create(FloatLiteral::Create(default_val));
}

BasicConstructor *BasicConstructor::CreateStringConstructor(str default_val) {
  return BasicConstructor::Create(StringLiteral::Create(default_val));
}

BasicConstructor *BasicConstructor::CreateCharConstructor(uint8_t default_val) {
  return BasicConstructor::Create(CharLiteral::Create(default_val));
}

BasicConstructor *BasicConstructor::CreateArrayConstructor(vector<Literal *> default_val) {
  return BasicConstructor::Create(ArrayLiteral::Create(default_val));
}

CompTimeExpr *BasicConstructor::get_value() const { return _value; }

void BasicConstructor::set_value(CompTimeExpr *val) { _value = val; }

StructConstructor::StructConstructor(ASTType *struct_type)
    : Constructor(ConstructorType::STRUCT), _struct_type(struct_type) {}

StructConstructor *StructConstructor::Create(ASTType *struct_type) {
  return new StructConstructor(struct_type);
}

StructConstructor *StructConstructor::Create(ASTType *struct_type, vector<Constructor *> member_ctrs) {
  auto *ret = new StructConstructor(struct_type);
  ret->_member_constructors = member_ctrs;
  return ret;
}

vector<Constructor *> &StructConstructor::get_member_constructors() { return _member_constructors; }

void StructConstructor::set_member_constructors(const vector<Constructor *> &member_constructors) {
  _member_constructors = member_constructors;
}

ASTType *StructConstructor::get_struct_type() const { return _struct_type; }
