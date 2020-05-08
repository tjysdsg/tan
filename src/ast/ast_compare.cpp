#include "src/ast/ast_compare.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "src/type_system.h"
#include "src/common.h"
#include "token.h"

namespace tanlang {

size_t ASTCompare::led(const ASTNodePtr &left) {
  auto ret = ASTInfixBinaryOp::led(left);
  _ty = ASTTy::Create(Ty::BOOL, std::vector<ASTNodePtr>());
  return ret;
}

Value *ASTCompare::codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  Value *lhs = _children[0]->codegen(cs);
  Value *rhs = _children[1]->codegen(cs);
  TAN_ASSERT(lhs && rhs);
  if (_children[0]->is_lvalue()) { lhs = cs->get_builder()->CreateLoad(lhs); }
  if (_children[1]->is_lvalue()) { rhs = cs->get_builder()->CreateLoad(rhs); }

  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();

  TAN_ASSERT(_children.size() > _dominant_idx);
  if (_dominant_idx == 0) {
    rhs = TypeSystem::ConvertTo(cs, ltype, rhs, false, true);
  } else {
    lhs = TypeSystem::ConvertTo(cs, rtype, lhs, false, true);
  }

  if (lhs->getType()->isFloatingPointTy()) {
    if (_type == ASTType::EQ) {
      _llvm_value = cs->get_builder()->CreateFCmpOEQ(lhs, rhs, "eq");
    } else if (_type == ASTType::NE) {
      _llvm_value = cs->get_builder()->CreateFCmpONE(lhs, rhs, "ne");
    } else if (_type == ASTType::GT) {
      _llvm_value = cs->get_builder()->CreateFCmpOGT(lhs, rhs, "gt");
    } else if (_type == ASTType::GE) {
      _llvm_value = cs->get_builder()->CreateFCmpOGE(lhs, rhs, "ge");
    } else if (_type == ASTType::LT) {
      _llvm_value = cs->get_builder()->CreateFCmpOLT(lhs, rhs, "lt");
    } else if (_type == ASTType::LE) {
      _llvm_value = cs->get_builder()->CreateFCmpOLE(lhs, rhs, "le");
    }
  } else {
    if (_type == ASTType::EQ) {
      _llvm_value = cs->get_builder()->CreateICmpEQ(lhs, rhs, "eq");
    } else if (_type == ASTType::NE) {
      _llvm_value = cs->get_builder()->CreateICmpNE(lhs, rhs, "ne");
    } else if (_type == ASTType::GT) {
      _llvm_value = cs->get_builder()->CreateICmpUGT(lhs, rhs, "gt");
    } else if (_type == ASTType::GE) {
      _llvm_value = cs->get_builder()->CreateICmpUGE(lhs, rhs, "ge");
    } else if (_type == ASTType::LT) {
      _llvm_value = cs->get_builder()->CreateICmpULT(lhs, rhs, "lt");
    } else if (_type == ASTType::LE) {
      _llvm_value = cs->get_builder()->CreateICmpULE(lhs, rhs, "le");
    }
  }
  return _llvm_value;
}

ASTCompare::ASTCompare(ASTType type, Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  if (!is_ast_type_in(type,
      {ASTType::GT, ASTType::GE, ASTType::LT, ASTType::LE, ASTType::LAND, ASTType::LNOT, ASTType::LOR, ASTType::EQ,
          ASTType::NE})) {
    report_code_error(token, "Invalid comparison: " + token->to_string());
  }
  _type = type;
  _lbp = op_precedence[type];
}

} // namespace tanlang
