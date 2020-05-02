#include "src/ast/ast_arithmetic.h"
#include "src/ast/ast_ty.h"
#include "src/type_system.h"
#include "src/common.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

Value *ASTArithmetic::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  if (_children.size() == 1) { /// unary plus/minus
    if (!is_ast_type_in(_type, {ASTType::SUM, ASTType::SUBTRACT})) {
      report_code_error(_token, "Invalid unary operation");
    }
    if (_type == ASTType::SUM) {
      return _children[0]->codegen(compiler_session);
    } else {
      auto *rhs = _children[0]->codegen(compiler_session);
      if (rhs->getType()->isFloatingPointTy()) {
        return compiler_session->get_builder()->CreateFNeg(rhs);
      }
      return compiler_session->get_builder()->CreateNeg(rhs);
    }
  }
  Value *lhs = _children[0]->codegen(compiler_session);
  Value *rhs = _children[1]->codegen(compiler_session);
  TAN_ASSERT(lhs && rhs);
  if (_children[0]->is_lvalue()) { lhs = compiler_session->get_builder()->CreateLoad(lhs); }
  if (_children[1]->is_lvalue()) { rhs = compiler_session->get_builder()->CreateLoad(rhs); }

  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();

  TAN_ASSERT(_children.size() > _dominant_idx);
  if (_dominant_idx == 0) {
    rhs = TypeSystem::ConvertTo(compiler_session, ltype, rhs, false, true);
  } else {
    lhs = TypeSystem::ConvertTo(compiler_session, rtype, lhs, false, true);
  }

  if (lhs->getType()->isFloatingPointTy()) {
    /// float arithmetic
    if (_type == ASTType::MULTIPLY) {
      return compiler_session->get_builder()->CreateFMul(lhs, rhs);
    } else if (_type == ASTType::DIVIDE) {
      return compiler_session->get_builder()->CreateFDiv(lhs, rhs);
    } else if (_type == ASTType::SUM) {
      return compiler_session->get_builder()->CreateFAdd(lhs, rhs);
    } else if (_type == ASTType::SUBTRACT) {
      return compiler_session->get_builder()->CreateFSub(lhs, rhs);
    }
  }
  /// integer arithmetic
  if (_type == ASTType::MULTIPLY) {
    return compiler_session->get_builder()->CreateMul(lhs, rhs, "mul_tmp");
  } else if (_type == ASTType::DIVIDE) {
    auto ty = _children[0]->get_ty();
    if (ty->is_unsigned()) {
      return compiler_session->get_builder()->CreateUDiv(lhs, rhs, "div_tmp");
    }
    return compiler_session->get_builder()->CreateSDiv(lhs, rhs, "div_tmp");
  } else if (_type == ASTType::SUM) {
    return compiler_session->get_builder()->CreateAdd(lhs, rhs, "sum_tmp");
  } else if (_type == ASTType::SUBTRACT) {
    return compiler_session->get_builder()->CreateSub(lhs, rhs, "sub_tmp");
  }
  TAN_ASSERT(false);
  return nullptr;
}

size_t ASTArithmetic::nud() {
  _end_index = _start_index + 1; /// skip "-" or "+"
  /// unary plus/minus has higher precedence than infix plus/minus
  _rbp = PREC_UNARY;
  _children.push_back(_parser->next_expression(_end_index, PREC_UNARY));
  return _end_index;
}

ASTArithmetic::ASTArithmetic(ASTType type, Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  if (!is_ast_type_in(type, {ASTType::SUM, ASTType::SUBTRACT, ASTType::MULTIPLY, ASTType::DIVIDE, ASTType::MOD})) {
    report_code_error(token, "Invalid ASTType for comparisons " + token->to_string());
  }
  _type = type;
  _lbp = op_precedence[type];
}

} // namespace tanlang
