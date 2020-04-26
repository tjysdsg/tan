#include "src/ast/ast_arithmetic.h"
#include "src/ast/type_system.h"
#include "src/ast/common.h"
#include "compiler_session.h"

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
  assert(lhs && rhs);
  if (_children[0]->is_lvalue()) {
    lhs = compiler_session->get_builder()->CreateLoad(lhs);
  }
  if (_children[1]->is_lvalue()) {
    rhs = compiler_session->get_builder()->CreateLoad(rhs);
  }

  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();

  int type_i = should_cast_to_which(ltype, rtype);
  if (type_i == -1) {
    report_code_error(_token,
        "Cannot compare " + _children[0]->get_type_name() + " and " + _children[1]->get_type_name());
  } else if (type_i == 0) {
    rhs = convert_to(compiler_session, ltype, rhs, false, true);
  } else {
    lhs = convert_to(compiler_session, rtype, lhs, false, true);
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
    // TODO: check if value is unsigned
    return compiler_session->get_builder()->CreateSDiv(lhs, rhs, "div_tmp");
  } else if (_type == ASTType::SUM) {
    return compiler_session->get_builder()->CreateAdd(lhs, rhs, "sum_tmp");
  } else if (_type == ASTType::SUBTRACT) {
    return compiler_session->get_builder()->CreateSub(lhs, rhs, "sub_tmp");
  }
  assert(false);
}

/// special case for parsing negative number
size_t ASTArithmetic::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip "-" or "+"
  /// unary plus/minus has higher precedence than infix plus/minus
  _rbp = PREC_UNARY;
  _children.push_back(parser->next_expression(_end_index, PREC_UNARY));
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
