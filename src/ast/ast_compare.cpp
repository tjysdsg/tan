#include "src/ast/ast_compare.h"
#include "src/ast/type_system.h"
#include "src/ast/common.h"
#include "compiler_session.h"

namespace tanlang {

ASTCompare::ASTCompare(ASTType type, Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  if (!is_ast_type_in(type,
      {ASTType::GT, ASTType::GE, ASTType::LT, ASTType::LE, ASTType::LAND, ASTType::LNOT, ASTType::LOR, ASTType::EQ,
          ASTType::NE})) {
    report_code_error(token, "Invalid comparison: " + token->to_string());
  }
  _type = type;
  _lbp = op_precedence[type];
}

Value *ASTCompare::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
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
    if (_type == ASTType::EQ) {
      return compiler_session->get_builder()->CreateFCmpOEQ(lhs, rhs, "eq");
    } else if (_type == ASTType::NE) {
      return compiler_session->get_builder()->CreateFCmpONE(lhs, rhs, "ne");
    } else if (_type == ASTType::GT) {
      return compiler_session->get_builder()->CreateFCmpOGT(lhs, rhs, "gt");
    } else if (_type == ASTType::GE) {
      return compiler_session->get_builder()->CreateFCmpOGE(lhs, rhs, "ge");
    } else if (_type == ASTType::LT) {
      return compiler_session->get_builder()->CreateFCmpOLT(lhs, rhs, "lt");
    } else if (_type == ASTType::LE) {
      return compiler_session->get_builder()->CreateFCmpOLE(lhs, rhs, "le");
    }
  }
  if (_type == ASTType::EQ) {
    return compiler_session->get_builder()->CreateICmpEQ(lhs, rhs, "eq");
  } else if (_type == ASTType::NE) {
    return compiler_session->get_builder()->CreateICmpNE(lhs, rhs, "ne");
  } else if (_type == ASTType::GT) {
    return compiler_session->get_builder()->CreateICmpUGT(lhs, rhs, "gt");
  } else if (_type == ASTType::GE) {
    return compiler_session->get_builder()->CreateICmpUGE(lhs, rhs, "ge");
  } else if (_type == ASTType::LT) {
    return compiler_session->get_builder()->CreateICmpULT(lhs, rhs, "lt");
  } else if (_type == ASTType::LE) {
    return compiler_session->get_builder()->CreateICmpULE(lhs, rhs, "le");
  }
  assert(false);
}

} // namespace tanlang
