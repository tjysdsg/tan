#include "src/ast/ast_builder.h"
#include "src/ast/expr.h"
#include "src/ast/ast_type.h"
#include "src/analysis/type_system.h"

using namespace tanlang;

IntegerLiteral *ASTBuilder::CreateIntegerLiteral(ASTContext *ctx,
    SourceIndex loc,
    uint64_t val,
    size_t bit_size,
    bool is_unsigned) {
  auto *ret = IntegerLiteral::Create(loc, val, is_unsigned);
  ASTType *ty = ASTType::CreateAndResolve(ctx, loc, TY_OR(Ty::INT, BIT_SIZE_TO_TY[bit_size]), {}, false);
  ret->set_type(ty);
  return ret;
}

BoolLiteral *ASTBuilder::CreateBoolLiteral(ASTContext *ctx, SourceIndex loc, bool val) {
  auto *ret = BoolLiteral::Create(loc, val);
  ASTType *ty = ASTType::CreateAndResolve(ctx, loc, Ty::BOOL);
  ret->set_type(ty);
  return ret;
}

FloatLiteral *ASTBuilder::CreateFloatLiteral(ASTContext *ctx, SourceIndex loc, double val, size_t bit_size) {
  auto *ret = FloatLiteral::Create(loc, val);
  if (bit_size == 32) {
    ret->set_type(ASTType::CreateAndResolve(ctx, loc, Ty::FLOAT));
  } else if (bit_size == 64) {
    ret->set_type(ASTType::CreateAndResolve(ctx, loc, Ty::DOUBLE));
  } else {
    TAN_ASSERT(false);
  }
  return ret;
}

StringLiteral *ASTBuilder::CreateStringLiteral(ASTContext *ctx, SourceIndex loc, str val) {
  auto *ret = StringLiteral::Create(loc, val);
  ret->set_type(ASTType::CreateAndResolve(ctx, loc, Ty::STRING));
  return ret;
}

CharLiteral *ASTBuilder::CreateCharLiteral(ASTContext *ctx, SourceIndex loc, uint8_t val) {
  auto *ret = CharLiteral::Create(loc, val);
  ret->set_type(ASTType::CreateAndResolve(ctx, loc, Ty::CHAR));
  return ret;
}

ArrayLiteral *ASTBuilder::CreateArrayLiteral(ASTContext *ctx, SourceIndex loc, ASTType *element_type) {
  auto *ret = ArrayLiteral::Create(loc);
  vector<ASTType *> sub_types{};
  auto *type = ASTType::CreateAndResolve(ctx, loc, Ty::ARRAY, {element_type});
  TypeSystem::ResolveTy(ctx, type);
  ret->set_type(type);
  return ret;
}
