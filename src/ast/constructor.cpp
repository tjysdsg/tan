#include "ast/constructor.h"
#include "ast/expr.h"
#include "ast/ast_builder.h"

using namespace tanlang;

BasicConstructor *BasicConstructor::Create(CompTimeExpr *val) {
  auto *ret = new BasicConstructor();
  ret->_value = val;
  return ret;
}

BasicConstructor *BasicConstructor::CreateIntegerConstructor(SrcLoc loc, uint64_t default_val, size_t bit_size,
                                                             bool is_unsigned) {
  return BasicConstructor::Create(ASTBuilder::CreateIntegerLiteral(loc, default_val, bit_size, is_unsigned));
}

BasicConstructor *BasicConstructor::CreateBoolConstructor(SrcLoc loc, bool default_val) {
  return BasicConstructor::Create(ASTBuilder::CreateBoolLiteral(loc, default_val));
}

BasicConstructor *BasicConstructor::CreateFPConstructor(SrcLoc loc, double default_val, size_t bit_size) {
  return BasicConstructor::Create(ASTBuilder::CreateFloatLiteral(loc, default_val, bit_size));
}

BasicConstructor *BasicConstructor::CreateStringConstructor(SrcLoc loc, str default_val) {
  return BasicConstructor::Create(ASTBuilder::CreateStringLiteral(loc, default_val));
}

BasicConstructor *BasicConstructor::CreateCharConstructor(SrcLoc loc, uint8_t default_val) {
  return BasicConstructor::Create(ASTBuilder::CreateCharLiteral(loc, default_val));
}

BasicConstructor *BasicConstructor::CreateArrayConstructor(SrcLoc loc, Type *element_type) {
  return BasicConstructor::Create(ASTBuilder::CreateArrayLiteral(loc, element_type, 0));
}

BasicConstructor *BasicConstructor::CreateNullPointerConstructor(SrcLoc loc, Type *element_type) {
  return BasicConstructor::Create(ASTBuilder::CreateNullPointerLiteral(loc, element_type));
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
