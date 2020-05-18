#include "src/ast/ast_compare.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "src/type_system.h"
#include "src/common.h"
#include "token.h"

namespace tanlang {

size_t ASTCompare::led(const ASTNodePtr &left) {
  auto ret = ASTInfixBinaryOp::led(left);
  _ty = ASTTy::Create(Ty::BOOL, vector<ASTNodePtr>());
  return ret;
}

Value *ASTCompare::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(_token->l, _token->c);
  auto lhs = _children[0];
  auto rhs = _children[1];
  Value *l = _children[0]->codegen(cs);
  Value *r = _children[1]->codegen(cs);
  TAN_ASSERT(l && r);
  TAN_ASSERT(_children.size() > _dominant_idx);

  if (_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(cs, r, rhs->get_ty(), lhs->get_ty());
    l = TypeSystem::ConvertTo(cs, l, lhs->get_ty(), lhs->get_ty());
  } else {
    l = TypeSystem::ConvertTo(cs, l, lhs->get_ty(), rhs->get_ty());
    r = TypeSystem::ConvertTo(cs, r, rhs->get_ty(), rhs->get_ty());
  }

  if (l->getType()->isFloatingPointTy()) {
    if (_type == ASTType::EQ) {
      _llvm_value = builder->CreateFCmpOEQ(l, r, "eq");
    } else if (_type == ASTType::NE) {
      _llvm_value = builder->CreateFCmpONE(l, r, "ne");
    } else if (_type == ASTType::GT) {
      _llvm_value = builder->CreateFCmpOGT(l, r, "gt");
    } else if (_type == ASTType::GE) {
      _llvm_value = builder->CreateFCmpOGE(l, r, "ge");
    } else if (_type == ASTType::LT) {
      _llvm_value = builder->CreateFCmpOLT(l, r, "lt");
    } else if (_type == ASTType::LE) {
      _llvm_value = builder->CreateFCmpOLE(l, r, "le");
    }
  } else {
    if (_type == ASTType::EQ) {
      _llvm_value = builder->CreateICmpEQ(l, r, "eq");
    } else if (_type == ASTType::NE) {
      _llvm_value = builder->CreateICmpNE(l, r, "ne");
    } else if (_type == ASTType::GT) {
      _llvm_value = builder->CreateICmpUGT(l, r, "gt");
    } else if (_type == ASTType::GE) {
      _llvm_value = builder->CreateICmpUGE(l, r, "ge");
    } else if (_type == ASTType::LT) {
      _llvm_value = builder->CreateICmpULT(l, r, "lt");
    } else if (_type == ASTType::LE) {
      _llvm_value = builder->CreateICmpULE(l, r, "le");
    }
  }
  return _llvm_value;
}

ASTCompare::ASTCompare(ASTType type, Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  if (!is_ast_type_in(type,
      {ASTType::GT, ASTType::GE, ASTType::LT, ASTType::LE, ASTType::LAND, ASTType::LNOT, ASTType::LOR, ASTType::EQ,
          ASTType::NE})) {
    error("Invalid comparison: " + token->to_string());
  }
  _type = type;
  _lbp = op_precedence[type];
}

} // namespace tanlang
