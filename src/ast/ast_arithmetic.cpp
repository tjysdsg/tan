#include "src/ast/ast_arithmetic.h"
#include "src/ast/ast_ty.h"
#include "src/type_system.h"
#include "src/common.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

/// special case for unary '-' or '+'
size_t ASTArithmetic::nud() {
  _end_index = _start_index + 1; /// skip "-" or "+"
  /// unary plus/minus has higher precedence than infix plus/minus
  _rbp = PREC_UNARY;
  _children.push_back(_parser->next_expression(_end_index, PREC_UNARY));
  _dominant_idx = 0;
  _ty = _children[0]->get_ty();
  return _end_index;
}

Value *ASTArithmetic::codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  if (_children.size() == 1) { /// unary plus/minus
    if (!is_ast_type_in(_type, {ASTType::SUM, ASTType::SUBTRACT})) {
      report_code_error(_token, "Invalid unary operation");
    }
    if (_type == ASTType::SUM) {
      return _children[0]->codegen(cs);
    } else {
      auto *rhs = _children[0]->codegen(cs);
      if (_children[0]->is_lvalue()) { rhs = cs->get_builder()->CreateLoad(rhs); }
      if (rhs->getType()->isFloatingPointTy()) { return cs->get_builder()->CreateFNeg(rhs); }
      return cs->get_builder()->CreateNeg(rhs);
    }
  }
  Value *lhs = _children[0]->codegen(cs);
  Value *rhs = _children[1]->codegen(cs);
  TAN_ASSERT(lhs && rhs);
  if (_children[0]->is_lvalue()) { lhs = cs->get_builder()->CreateLoad(lhs); }
  if (_children[1]->is_lvalue()) { rhs = cs->get_builder()->CreateLoad(rhs); }

  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();

  TAN_ASSERT(_children.size() > _dominant_idx);
  if (_dominant_idx == 0) { rhs = TypeSystem::ConvertTo(cs, ltype, rhs, false, true); }
  else { lhs = TypeSystem::ConvertTo(cs, rtype, lhs, false, true); }

  if (lhs->getType()->isFloatingPointTy()) {
    /// float arithmetic
    if (_type == ASTType::MULTIPLY) {
      _llvm_value = cs->get_builder()->CreateFMul(lhs, rhs, "mul_tmp");
    } else if (_type == ASTType::DIVIDE) {
      _llvm_value = cs->get_builder()->CreateFDiv(lhs, rhs, "div_tmp");
    } else if (_type == ASTType::SUM) {
      _llvm_value = cs->get_builder()->CreateFAdd(lhs, rhs, "sum_tmp");
    } else if (_type == ASTType::SUBTRACT) {
      _llvm_value = cs->get_builder()->CreateFSub(lhs, rhs, "sub_tmp");
    } else if (_type == ASTType::MOD) {
      _llvm_value = cs->get_builder()->CreateFRem(lhs, rhs, "mod_tmp");
    } else { TAN_ASSERT(false); }
  } else {
    /// integer arithmetic
    if (_type == ASTType::MULTIPLY) {
      _llvm_value = cs->get_builder()->CreateMul(lhs, rhs, "mul_tmp");
    } else if (_type == ASTType::DIVIDE) {
      auto ty = _children[0]->get_ty();
      if (ty->is_unsigned()) { _llvm_value = cs->get_builder()->CreateUDiv(lhs, rhs, "div_tmp"); }
      else { _llvm_value = cs->get_builder()->CreateSDiv(lhs, rhs, "div_tmp"); }
    } else if (_type == ASTType::SUM) {
      _llvm_value = cs->get_builder()->CreateAdd(lhs, rhs, "sum_tmp");
    } else if (_type == ASTType::SUBTRACT) {
      _llvm_value = cs->get_builder()->CreateSub(lhs, rhs, "sub_tmp");
    } else if (_type == ASTType::MOD) {
      auto ty = _children[0]->get_ty();
      if (ty->is_unsigned()) { _llvm_value = cs->get_builder()->CreateURem(lhs, rhs, "mod_tmp"); }
      else { _llvm_value = cs->get_builder()->CreateSRem(lhs, rhs, "mod_tmp"); }
    } else { TAN_ASSERT(false); }
  }
  return _llvm_value;
}

ASTArithmetic::ASTArithmetic(Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  switch_str(token->value)
  case_str0("+") _type = ASTType::SUM;     //
  case_str("-") _type = ASTType::SUBTRACT; //
  case_str("*") _type = ASTType::MULTIPLY; //
  case_str("/") _type = ASTType::DIVIDE;   //
  case_str("%") _type = ASTType::MOD;      //
  case_default report_code_error(token, "Invalid ASTType for comparisons " + token->to_string()); //
  end_switch
  _lbp = op_precedence[_type];
}

} // namespace tanlang
