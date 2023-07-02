#include "ast/constructor.h"
#include "ast/expr.h"

using namespace tanlang;

BasicConstructor *BasicConstructor::Create(CompTimeExpr *val) {
  auto *ret = new BasicConstructor();
  ret->_value = val;
  return ret;
}

BasicConstructor *BasicConstructor::CreateIntegerConstructor(SourceFile *src, uint64_t default_val, size_t bit_size,
                                                             bool is_unsigned) {
  return BasicConstructor::Create(Literal::CreateIntegerLiteral(src, default_val, bit_size, is_unsigned));
}

BasicConstructor *BasicConstructor::CreateBoolConstructor(SourceFile *src, bool default_val) {
  return BasicConstructor::Create(Literal::CreateBoolLiteral(src, default_val));
}

BasicConstructor *BasicConstructor::CreateFPConstructor(SourceFile *src, double default_val, size_t bit_size) {
  return BasicConstructor::Create(Literal::CreateFloatLiteral(src, default_val, bit_size));
}

BasicConstructor *BasicConstructor::CreateStringConstructor(SourceFile *src, str default_val) {
  return BasicConstructor::Create(Literal::CreateStringLiteral(src, default_val));
}

BasicConstructor *BasicConstructor::CreateCharConstructor(SourceFile *src, uint8_t default_val) {
  return BasicConstructor::Create(Literal::CreateCharLiteral(src, default_val));
}

BasicConstructor *BasicConstructor::CreateArrayConstructor(SourceFile *src, Type *element_type) {
  return BasicConstructor::Create(Literal::CreateArrayLiteral(src, element_type, 0));
}

BasicConstructor *BasicConstructor::CreateNullPointerConstructor(SourceFile *src, Type *element_type) {
  return BasicConstructor::Create(Literal::CreateNullPointerLiteral(src, element_type));
}

CompTimeExpr *BasicConstructor::get_value() const { return _value; }

void BasicConstructor::set_value(CompTimeExpr *val) { _value = val; }

StructConstructor::StructConstructor(Type *struct_type)
    : Constructor(ConstructorType::STRUCT), _struct_type(struct_type) {}

StructConstructor *StructConstructor::Create(Type *struct_type) { return new StructConstructor(struct_type); }

StructConstructor *StructConstructor::Create(Type *struct_type, vector<Constructor *> member_ctrs) {
  auto *ret = new StructConstructor(struct_type);
  ret->_member_constructors = std::move(member_ctrs);
  return ret;
}

vector<Constructor *> &StructConstructor::get_member_constructors() { return _member_constructors; }

void StructConstructor::set_member_constructors(const vector<Constructor *> &member_constructors) {
  _member_constructors = member_constructors;
}

Type *StructConstructor::get_struct_type() const { return _struct_type; }
