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
  std::string type_name = std::reinterpret_pointer_cast<ASTTypeName>(_children[1])->_name;
  Type *type = typename_to_llvm_type(type_name, compiler_session);
  Value *var = create_block_alloca(compiler_session->get_builder()->GetInsertBlock(), type, name);

  // set initial value
  if (_has_initial_val) {
    compiler_session->get_builder()->CreateStore(_children[2]->codegen(compiler_session), var);
  }

  // add to current scope
  // NOTE: adding to scope AFTER setting the intial value allows initialization like:
  //     var a = a;
  //     where the latter `a` refers to outer a variable in the outer scope.
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
  if (!rhs) {
    assert(false);
  }
  return compiler_session->get_builder()->CreateNot(rhs);
}

Value *ASTLogicalNot::codegen(CompilerSession *compiler_session) {
  auto *rhs = _children[0]->codegen(compiler_session);
  if (!rhs) {
    assert(false);
  }
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

    if (_op == ASTType::GT) {
      return compiler_session->get_builder()->CreateFCmpOGT(lhs, rhs);
    } else if (_op == ASTType::GE) {
      return compiler_session->get_builder()->CreateFCmpOGE(lhs, rhs);
    } else if (_op == ASTType::LT) {
      return compiler_session->get_builder()->CreateFCmpOLT(lhs, rhs);
    } else if (_op == ASTType::LE) {
      return compiler_session->get_builder()->CreateFCmpOLE(lhs, rhs);
    }
  }
  if (_op == ASTType::GT) {
    return compiler_session->get_builder()->CreateICmpUGT(lhs, rhs);
  } else if (_op == ASTType::GE) {
    return compiler_session->get_builder()->CreateICmpUGE(lhs, rhs);
  } else if (_op == ASTType::LT) {
    return compiler_session->get_builder()->CreateICmpULT(lhs, rhs);
  } else if (_op == ASTType::LE) {
    return compiler_session->get_builder()->CreateICmpULE(lhs, rhs);
  }
  return nullptr;
}

Value *ASTArithmetic::codegen(CompilerSession *compiler_session) {
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
    // float arithmetic
    if (_op == ASTType::MULTIPLY) {
      return compiler_session->get_builder()->CreateFMul(lhs, rhs);
    } else if (_op == ASTType::DIVIDE) {
      return compiler_session->get_builder()->CreateFDiv(lhs, rhs);
    } else if (_op == ASTType::SUM) {
      return compiler_session->get_builder()->CreateFAdd(lhs, rhs);
    } else if (_op == ASTType::SUBTRACT) {
      return compiler_session->get_builder()->CreateFSub(lhs, rhs);
    }
  }

  // integer arithmetic
  if (_op == ASTType::MULTIPLY) {
    return compiler_session->get_builder()->CreateMul(lhs, rhs, "mul_tmp");
  } else if (_op == ASTType::DIVIDE) {
    return compiler_session->get_builder()->CreateUDiv(lhs, rhs, "div_tmp");
  } else if (_op == ASTType::SUM) {
    return compiler_session->get_builder()->CreateAdd(lhs, rhs, "sum_tmp");
  } else if (_op == ASTType::SUBTRACT) {
    return compiler_session->get_builder()->CreateSub(lhs, rhs, "sub_tmp");
  }
  return nullptr;
}

Value *ASTAssignment::codegen(CompilerSession *compiler_session) {
  // Assignment requires the lhs to be an mutable variable.
  auto lhs = std::reinterpret_pointer_cast<ASTIdentifier>(_children[0]);
  if (!lhs) {
    report_code_error(lhs->_token->l, lhs->_token->c, "Left-hand operand of assignment must be a variable");
  }
  // codegen the rhs
  auto rhs = _children[1];
  Value *val = rhs->codegen(compiler_session);
  if (!val) {
    report_code_error(rhs->_token->l, rhs->_token->c, "Invalid expression for right-hand operand of the assignment");
  }

  // look up variable by name
  Value *variable = compiler_session->get(lhs->_name);
  if (!variable) {
    report_code_error(lhs->_token->l, lhs->_token->c, "Invalid variable name");
  }

  // TODO: check type
  // TODO: implicit type conversion
  compiler_session->get_builder()->CreateStore(val, variable);
  return val;
}

/// \attention UNUSED
Value *ASTArgDecl::codegen(CompilerSession *compiler_session) {
  UNUSED(compiler_session);
  assert(false);
}

}
