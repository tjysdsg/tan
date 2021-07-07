#include "src/ast/ast_builder.h"
#include "src/ast/expr.h"
#include "src/ast/ast_type.h"
#include "src/analysis/type_system.h"

using namespace tanlang;

IntegerLiteral *ASTBuilder::CreateIntegerLiteral(CompilerSession *cs, uint64_t val, size_t bit_size, bool is_unsigned) {
  auto *ret = IntegerLiteral::Create(val, is_unsigned);
  ASTType *ty = ASTType::CreateAndResolve(cs, TY_OR(Ty::INT, BIT_SIZE_TO_TY[bit_size]), {}, false);
  ret->set_type(ty);
  return ret;
}

FloatLiteral *ASTBuilder::CreateFloatLiteral(CompilerSession *cs, double val) {
  auto *ret = FloatLiteral::Create(val);
  ret->set_type(ASTType::CreateAndResolve(cs, Ty::FLOAT));
  return ret;
}

StringLiteral *ASTBuilder::CreateStringLiteral(CompilerSession *cs, str val) {
  auto *ret = StringLiteral::Create(val);
  ret->set_type(ASTType::CreateAndResolve(cs, Ty::STRING));
  return ret;
}

CharLiteral *ASTBuilder::CreateCharLiteral(CompilerSession *cs, uint8_t val) {
  auto *ret = CharLiteral::Create(val);
  ret->set_type(ASTType::CreateAndResolve(cs, Ty::CHAR));
  return ret;
}

ArrayLiteral *ASTBuilder::CreateArrayLiteral(CompilerSession *cs, ASTType *element_type) {
  auto *ret = ArrayLiteral::Create();
  vector<ASTType *> sub_types{};
  auto *type = ASTType::CreateAndResolve(cs, Ty::ARRAY, {element_type});
  TypeSystem::ResolveTy(cs, type);
  ret->set_type(type);
  return ret;
}
