#include "src/ast/ast_builder.h"
#include "src/ast/expr.h"
#include "src/ast/ast_type.h"
#include "src/analysis/type_system.h"

using namespace tanlang;

IntegerLiteral *ASTBuilder::CreateIntegerLiteral(CompilerSession *cs,
    SourceIndex loc,
    uint64_t val,
    size_t bit_size,
    bool is_unsigned) {
  auto *ret = IntegerLiteral::Create(val, is_unsigned);
  ASTType *ty = ASTType::CreateAndResolve(cs, loc, TY_OR(Ty::INT, BIT_SIZE_TO_TY[bit_size]), {}, false);
  ret->set_type(ty);
  return ret;
}

FloatLiteral *ASTBuilder::CreateFloatLiteral(CompilerSession *cs, SourceIndex loc, double val, size_t bit_size) {
  auto *ret = FloatLiteral::Create(val);
  if (bit_size == 32) {
    ret->set_type(ASTType::CreateAndResolve(cs, loc, Ty::FLOAT));
  } else if (bit_size == 64) {
    ret->set_type(ASTType::CreateAndResolve(cs, loc, Ty::DOUBLE));
  } else {
    TAN_ASSERT(false);
  }
  return ret;
}

StringLiteral *ASTBuilder::CreateStringLiteral(CompilerSession *cs, SourceIndex loc, str val) {
  auto *ret = StringLiteral::Create(val);
  ret->set_type(ASTType::CreateAndResolve(cs, loc, Ty::STRING));
  return ret;
}

CharLiteral *ASTBuilder::CreateCharLiteral(CompilerSession *cs, SourceIndex loc, uint8_t val) {
  auto *ret = CharLiteral::Create(val);
  ret->set_type(ASTType::CreateAndResolve(cs, loc, Ty::CHAR));
  return ret;
}

ArrayLiteral *ASTBuilder::CreateArrayLiteral(CompilerSession *cs, SourceIndex loc, ASTType *element_type) {
  auto *ret = ArrayLiteral::Create();
  vector<ASTType *> sub_types{};
  auto *type = ASTType::CreateAndResolve(cs, loc, Ty::ARRAY, {element_type});
  TypeSystem::ResolveTy(cs, type);
  ret->set_type(type);
  return ret;
}
