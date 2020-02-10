#include "src/ast/ast_expr.h"
#include "token.h"
#include "common.h"
#include "parser.h"

namespace tanlang {

Value *ASTParenthesis::codegen(CompilerSession *compiler_session) {
  auto *result = _children[0]->codegen(compiler_session);
  size_t n = _children.size();
  for (size_t i = 1; i < n; ++i) {
    _children[i]->codegen(compiler_session);
  }
  return result;
}

Value *ASTVarDecl::codegen(CompilerSession *compiler_session) {
  std::string name = std::reinterpret_pointer_cast<ASTIdentifier>(_children[0])->_name;
  auto ast_ty = std::reinterpret_pointer_cast<ASTTy>(_children[1]);
  Type *type = ast_ty->to_llvm_type(compiler_session);
  Value *var = create_block_alloca(compiler_session->get_builder()->GetInsertBlock(), type, name);

  // set initial value
  if (_has_initial_val) {
    compiler_session->get_builder()->CreateStore(_children[2]->codegen(compiler_session), var);
  }

  /**
   * add to current scope
   * \NOTE: adding to scope AFTER setting the initial value allows initialization like:
   *     var a = a;
   *     where the latter `a` refers to outer a variable in the outer scope.
   * */
  compiler_session->add(name, var);

  return nullptr;
}

Value *ASTNumberLiteral::codegen(CompilerSession *compiler_session) {
  if (_is_float) {
    return ConstantFP::get(*compiler_session->get_context(), APFloat(_fvalue));
  } else {
    return ConstantInt::get(*compiler_session->get_context(), APInt(32, static_cast<uint64_t>(_ivalue), true));
  }
}

Value *ASTBinaryNot::codegen(CompilerSession *compiler_session) {
  auto *rhs = _children[0]->codegen(compiler_session);
  if (!rhs) { assert(false); }
  return compiler_session->get_builder()->CreateNot(rhs);
}

Value *ASTLogicalNot::codegen(CompilerSession *compiler_session) {
  auto *rhs = _children[0]->codegen(compiler_session);
  if (!rhs) { assert(false); }
  // get value size in bits
  llvm::DataLayout data_layout(compiler_session->get_module().get());
  auto size_in_bits = data_layout.getTypeSizeInBits(rhs->getType());
  auto mask = ConstantInt::get(*compiler_session->get_context(),
                               APInt(static_cast<uint32_t>(size_in_bits), std::numeric_limits<uint64_t>::max(), false));
  return compiler_session->get_builder()->CreateXor(mask, rhs);
}

Value *ASTCompare::codegen(CompilerSession *compiler_session) {
  Value *lhs = _children[0]->codegen(compiler_session);
  Value *rhs = _children[1]->codegen(compiler_session);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }

  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();
  Type *float_type = compiler_session->get_builder()->getFloatTy();
  if (ltype->isPointerTy()) {
    lhs = compiler_session->get_builder()->CreateLoad(float_type, lhs);
  }
  if (rtype->isPointerTy()) {
    rhs = compiler_session->get_builder()->CreateLoad(float_type, rhs);
  }
  if (!ltype->isIntegerTy() || !rtype->isIntegerTy()) {
    if (ltype->isIntegerTy()) {
      lhs = compiler_session->get_builder()->CreateSIToFP(lhs, float_type);
    }
    if (rtype->isIntegerTy()) {
      rhs = compiler_session->get_builder()->CreateSIToFP(rhs, float_type);
    }

    if (_type == ASTType::GT) {
      return compiler_session->get_builder()->CreateFCmpOGT(lhs, rhs);
    } else if (_type == ASTType::GE) {
      return compiler_session->get_builder()->CreateFCmpOGE(lhs, rhs);
    } else if (_type == ASTType::LT) {
      return compiler_session->get_builder()->CreateFCmpOLT(lhs, rhs);
    } else if (_type == ASTType::LE) {
      return compiler_session->get_builder()->CreateFCmpOLE(lhs, rhs);
    }
  }
  if (_type == ASTType::GT) {
    return compiler_session->get_builder()->CreateICmpUGT(lhs, rhs);
  } else if (_type == ASTType::GE) {
    return compiler_session->get_builder()->CreateICmpUGE(lhs, rhs);
  } else if (_type == ASTType::LT) {
    return compiler_session->get_builder()->CreateICmpULT(lhs, rhs);
  } else if (_type == ASTType::LE) {
    return compiler_session->get_builder()->CreateICmpULE(lhs, rhs);
  }
  return nullptr;
}

Value *ASTArithmetic::codegen(CompilerSession *compiler_session) {
  Value *lhs = _children[0]->codegen(compiler_session);
  Value *rhs = _children[1]->codegen(compiler_session);
  if (!lhs || !rhs) { assert(false); }
  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();
  Type *float_type = compiler_session->get_builder()->getFloatTy();

  if (ltype->isPointerTy()) {
    lhs = compiler_session->get_builder()->CreateLoad(float_type, lhs);
  }
  if (rtype->isPointerTy()) {
    rhs = compiler_session->get_builder()->CreateLoad(float_type, rhs);
  }
  if (!ltype->isIntegerTy() || !rtype->isIntegerTy()) {
    if (ltype->isIntegerTy()) {
      lhs = compiler_session->get_builder()->CreateSIToFP(lhs, float_type);
    }
    if (rtype->isIntegerTy()) {
      rhs = compiler_session->get_builder()->CreateSIToFP(rhs, float_type);
    }
    // float arithmetic
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

  // integer arithmetic
  if (_type == ASTType::MULTIPLY) {
    return compiler_session->get_builder()->CreateMul(lhs, rhs, "mul_tmp");
  } else if (_type == ASTType::DIVIDE) {
    return compiler_session->get_builder()->CreateUDiv(lhs, rhs, "div_tmp");
  } else if (_type == ASTType::SUM) {
    return compiler_session->get_builder()->CreateAdd(lhs, rhs, "sum_tmp");
  } else if (_type == ASTType::SUBTRACT) {
    return compiler_session->get_builder()->CreateSub(lhs, rhs, "sub_tmp");
  }
  return nullptr;
}

Value *ASTAssignment::codegen(CompilerSession *compiler_session) {
  // assignment requires the lhs to be an mutable variable.
  auto lhs = std::reinterpret_pointer_cast<ASTIdentifier>(_children[0]);
  if (!lhs) {
    report_code_error(lhs->_token, "Left-hand operand of assignment must be a variable");
  }
  // codegen the rhs
  auto rhs = _children[1];
  Value *val = rhs->codegen(compiler_session);
  if (!val) {
    report_code_error(rhs->_token, "Invalid expression for right-hand operand of the assignment");
  }

  // look up variable by name
  Value *variable = compiler_session->get(lhs->_name);
  if (!variable) {
    report_code_error(lhs->_token, "Invalid variable name");
  }

  // TODO: check type
  // TODO: implicit type conversion
  compiler_session->get_builder()->CreateStore(val, variable);
  return val;
}

Value *ASTArgDecl::codegen(CompilerSession *compiler_session) {
  UNUSED(compiler_session);
  assert(false);
}

ASTNumberLiteral::ASTNumberLiteral(const std::string &str, bool is_float, Token *token) : ASTNode(ASTType::NUM_LITERAL,
                                                                                                  op_precedence[ASTType::NUM_LITERAL],
                                                                                                  0, token) {
  _is_float = is_float;
  if (is_float) {
    _fvalue = std::stof(str);
  } else {
    _ivalue = std::stoi(str);
  }
}

ASTStringLiteral::ASTStringLiteral(std::string str, Token *token) : ASTNode(ASTType::STRING_LITERAL,
                                                                            op_precedence[ASTType::STRING_LITERAL],
                                                                            0, token), _svalue(std::move(str)) {}

ASTAssignment::ASTAssignment(Token *token) : ASTInfixBinaryOp(token) {
  _type = ASTType::ASSIGN;
  _lbp = op_precedence[_type];
}

} // namespace tanlang
