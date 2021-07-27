#include "src/ast/constructor.h"
#include "src/ast/expr.h"
#include "src/ast/ast_builder.h"

using namespace tanlang;

BasicConstructor *BasicConstructor::Create(CompTimeExpr *val) {
  auto *ret = new BasicConstructor();
  ret->_value = val;
  return ret;
}

BasicConstructor *BasicConstructor::CreateIntegerConstructor(ASTContext *ctx,
    SourceIndex loc,
    uint64_t default_val,
    size_t bit_size,
    bool is_unsigned) {
  return BasicConstructor::Create(ASTBuilder::CreateIntegerLiteral(ctx, loc, default_val, bit_size, is_unsigned));
}

BasicConstructor *BasicConstructor::CreateBoolConstructor(ASTContext *ctx, SourceIndex loc, bool default_val) {
  return BasicConstructor::Create(ASTBuilder::CreateBoolLiteral(ctx, loc, default_val));
}

BasicConstructor *BasicConstructor::CreateFPConstructor(ASTContext *ctx,
    SourceIndex loc,
    double default_val,
    size_t bit_size) {
  return BasicConstructor::Create(ASTBuilder::CreateFloatLiteral(ctx, loc, default_val, bit_size));
}

BasicConstructor *BasicConstructor::CreateStringConstructor(ASTContext *ctx, SourceIndex loc, str default_val) {
  return BasicConstructor::Create(ASTBuilder::CreateStringLiteral(ctx, loc, default_val));
}

BasicConstructor *BasicConstructor::CreateCharConstructor(ASTContext *ctx, SourceIndex loc, uint8_t default_val) {
  return BasicConstructor::Create(ASTBuilder::CreateCharLiteral(ctx, loc, default_val));
}

BasicConstructor *BasicConstructor::CreateArrayConstructor(ASTContext *ctx, SourceIndex loc, ASTType *element_type) {
  return BasicConstructor::Create(ASTBuilder::CreateArrayLiteral(ctx, loc, element_type));
}

BasicConstructor *BasicConstructor::CreateNullPointerConstructor(ASTContext *ctx,
    SourceIndex loc,
    ASTType *element_type) {
  return BasicConstructor::Create(ASTBuilder::CreateNullPointerLiteral(ctx, loc, element_type));
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
  ret->_member_constructors = std::move(member_ctrs);
  return ret;
}

vector<Constructor *> &StructConstructor::get_member_constructors() { return _member_constructors; }

void StructConstructor::set_member_constructors(const vector<Constructor *> &member_constructors) {
  _member_constructors = member_constructors;
}

ASTType *StructConstructor::get_struct_type() const { return _struct_type; }
