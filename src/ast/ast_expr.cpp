#include "src/ast/ast_expr.h"
#include "token.h"
#include "common.h"
#include "parser.h"

namespace tanlang {

Value *ASTParenthesis::codegen(ParserContext *parser_context) {
  auto *result = _children[0]->codegen(parser_context);
  size_t n = _children.size();
  for (size_t i = 1; i < n; ++i) {
    _children[i]->codegen(parser_context);
  }
  return result;
}

Value *ASTVarDecl::codegen(ParserContext *parser_context) {
  std::string name = std::reinterpret_pointer_cast<ASTIdentifier>(_children[0])->_name;
  std::string type_name = std::reinterpret_pointer_cast<ASTTypeName>(_children[1])->_name;
  Type *type = typename_to_llvm_type(type_name, parser_context);
  Value *var = create_block_alloca(parser_context->_builder->GetInsertBlock(), type, name);

  // set initial value
  if (_has_initial_val) {
    parser_context->_builder->CreateStore(_children[2]->codegen(parser_context), var);
  }

  // add to current scope
  // NOTE: adding to scope AFTER setting the intial value allows initialization like:
  //     var a = a;
  //     where the latter `a` refers to outer a variable in the outer scope.
  parser_context->add(name, var);

  return nullptr;
}

Value *ASTNumberLiteral::codegen(ParserContext *parser_context) {
  if (_is_float) {
    return ConstantFP::get(*parser_context->_context, APFloat(_fvalue));
  } else {
    return ConstantInt::get(*parser_context->_context, APInt(32, static_cast<uint64_t>(_ivalue), true));
  }
}

Value *ASTBinaryNot::codegen(ParserContext *parser_context) {
  auto *rhs = _children[0]->codegen(parser_context);
  if (!rhs) {
    assert(false);
  }
  return parser_context->_builder->CreateNot(rhs);
}

Value *ASTLogicalNot::codegen(ParserContext *parser_context) {
  auto *rhs = _children[0]->codegen(parser_context);
  if (!rhs) {
    assert(false);
  }
  // get value size in bits
  llvm::DataLayout data_layout(parser_context->_module.get());
  auto size_in_bits = data_layout.getTypeSizeInBits(rhs->getType());
  auto mask = ConstantInt::get(*parser_context->_context,
                               APInt(static_cast<uint32_t>(size_in_bits), std::numeric_limits<uint64_t>::max(), false));
  return parser_context->_builder->CreateXor(mask, rhs);
}

Value *ASTCompare::codegen(ParserContext *parser_context) {
  Value *lhs = _children[0]->codegen(parser_context);
  Value *rhs = _children[1]->codegen(parser_context);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }

  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();
  Type *float_type = parser_context->_builder->getFloatTy();
  if (ltype->isPointerTy()) {
    lhs = parser_context->_builder->CreateLoad(float_type, lhs);
  }
  if (rtype->isPointerTy()) {
    rhs = parser_context->_builder->CreateLoad(float_type, rhs);
  }
  if (!ltype->isIntegerTy() || !rtype->isIntegerTy()) {
    if (ltype->isIntegerTy()) {
      lhs = parser_context->_builder->CreateSIToFP(lhs, float_type);
    }
    if (rtype->isIntegerTy()) {
      rhs = parser_context->_builder->CreateSIToFP(rhs, float_type);
    }

    if (_op == ASTType::GT) {
      return parser_context->_builder->CreateFCmpOGT(lhs, rhs);
    } else if (_op == ASTType::GE) {
      return parser_context->_builder->CreateFCmpOGE(lhs, rhs);
    } else if (_op == ASTType::LT) {
      return parser_context->_builder->CreateFCmpOLT(lhs, rhs);
    } else if (_op == ASTType::LE) {
      return parser_context->_builder->CreateFCmpOLE(lhs, rhs);
    }
  }
  if (_op == ASTType::GT) {
    return parser_context->_builder->CreateICmpUGT(lhs, rhs);
  } else if (_op == ASTType::GE) {
    return parser_context->_builder->CreateICmpUGE(lhs, rhs);
  } else if (_op == ASTType::LT) {
    return parser_context->_builder->CreateICmpULT(lhs, rhs);
  } else if (_op == ASTType::LE) {
    return parser_context->_builder->CreateICmpULE(lhs, rhs);
  }
  return nullptr;
}

Value *ASTArithmetic::codegen(ParserContext *parser_context) {
  Value *lhs = _children[0]->codegen(parser_context);
  Value *rhs = _children[1]->codegen(parser_context);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }
  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();
  Type *float_type = parser_context->_builder->getFloatTy();

  if (ltype->isPointerTy()) {
    lhs = parser_context->_builder->CreateLoad(float_type, lhs);
  }
  if (rtype->isPointerTy()) {
    rhs = parser_context->_builder->CreateLoad(float_type, rhs);
  }
  if (!ltype->isIntegerTy() || !rtype->isIntegerTy()) {
    if (ltype->isIntegerTy()) {
      lhs = parser_context->_builder->CreateSIToFP(lhs, float_type);
    }
    if (rtype->isIntegerTy()) {
      rhs = parser_context->_builder->CreateSIToFP(rhs, float_type);
    }
    // float arithmetic
    if (_op == ASTType::MULTIPLY) {
      return parser_context->_builder->CreateFMul(lhs, rhs);
    } else if (_op == ASTType::DIVIDE) {
      return parser_context->_builder->CreateFDiv(lhs, rhs);
    } else if (_op == ASTType::SUM) {
      return parser_context->_builder->CreateFAdd(lhs, rhs);
    } else if (_op == ASTType::SUBTRACT) {
      return parser_context->_builder->CreateFSub(lhs, rhs);
    }
  }

  // integer arithmetic
  if (_op == ASTType::MULTIPLY) {
    return parser_context->_builder->CreateMul(lhs, rhs, "mul_tmp");
  } else if (_op == ASTType::DIVIDE) {
    return parser_context->_builder->CreateUDiv(lhs, rhs, "div_tmp");
  } else if (_op == ASTType::SUM) {
    return parser_context->_builder->CreateAdd(lhs, rhs, "sum_tmp");
  } else if (_op == ASTType::SUBTRACT) {
    return parser_context->_builder->CreateSub(lhs, rhs, "sub_tmp");
  }
  return nullptr;
}

Value *ASTAssignment::codegen(ParserContext *parser_context) {
  // Assignment requires the lhs to be an mutable variable.
  auto lhs = std::reinterpret_pointer_cast<ASTIdentifier>(_children[0]);
  if (!lhs) {
    report_code_error(lhs->_token->l, lhs->_token->c, "Left-hand operand of assignment must be a variable");
  }
  // codegen the rhs
  auto rhs = _children[1];
  Value *val = rhs->codegen(parser_context);
  if (!val) {
    report_code_error(rhs->_token->l, rhs->_token->c, "Invalid expression for right-hand operand of the assignment");
  }

  // look up variable by name
  Value *variable = parser_context->get(lhs->_name);
  if (!variable) {
    report_code_error(lhs->_token->l, lhs->_token->c, "Invalid variable name");
  }

  // TODO: check type
  // TODO: implicit type conversion

  parser_context->_builder->CreateStore(val, variable);
  return val;
}

/// \attention UNUSED
Value *ASTArgDecl::codegen(ParserContext *parser_context) {
  UNUSED(parser_context);
  assert(false);
}

}
