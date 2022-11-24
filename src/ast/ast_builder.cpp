#include "src/ast/ast_builder.h"
#include "src/ast/expr.h"
#include "src/ast/type.h"

using namespace tanlang;

IntegerLiteral *ASTBuilder::CreateIntegerLiteral(SrcLoc loc, uint64_t val, size_t bit_size, bool is_unsigned) {
  auto *ret = IntegerLiteral::Create(loc, val, is_unsigned);
  auto *ty = Type::GetIntegerType(bit_size, is_unsigned);
  ret->set_type(ty);
  return ret;
}

BoolLiteral *ASTBuilder::CreateBoolLiteral(SrcLoc loc, bool val) {
  auto *ret = BoolLiteral::Create(loc, val);
  Type *ty = Type::GetBoolType();
  ret->set_type(ty);
  return ret;
}

FloatLiteral *ASTBuilder::CreateFloatLiteral(SrcLoc loc, double val, size_t bit_size) {
  auto *ret = FloatLiteral::Create(loc, val);
  ret->set_type(Type::GetFloatType(bit_size));
  return ret;
}

StringLiteral *ASTBuilder::CreateStringLiteral(SrcLoc loc, str val) {
  auto *ret = StringLiteral::Create(loc, val);
  ret->set_type(Type::GetStringType());
  return ret;
}

CharLiteral *ASTBuilder::CreateCharLiteral(SrcLoc loc, uint8_t val) {
  auto *ret = CharLiteral::Create(loc, val);
  ret->set_type(Type::GetCharType());
  return ret;
}

ArrayLiteral *ASTBuilder::CreateArrayLiteral(SrcLoc loc, Type *element_type, int size) {
  auto *ret = ArrayLiteral::Create(loc);
  vector<Type *> sub_types{};
  auto *type = Type::GetArrayType(element_type, size);
  ret->set_type(type);
  return ret;
}

NullPointerLiteral *ASTBuilder::CreateNullPointerLiteral(SrcLoc loc, Type *element_type) {
  auto *ret = NullPointerLiteral::Create(loc);
  auto *type = Type::GetPointerType(element_type);
  ret->set_type(type);
  return ret;
}
