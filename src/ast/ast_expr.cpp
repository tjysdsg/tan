#include "src/ast/ast_expr.h"
#include "token.h"
#include "common.h"
#include "parser.h"
#include "astnode.h"

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
  std::string name = ast_cast<ASTIdentifier>(_children[0])->get_name();
  Type *type = ast_cast<ASTTy>(_children[1])->to_llvm_type(compiler_session);
  Value *var = create_block_alloca(compiler_session->get_builder()->GetInsertBlock(), type, name);

  // set initial value
  if (_has_initial_val) {
    compiler_session->get_builder()->CreateStore(_children[2]->codegen(compiler_session), var);
  }
  this->_llvm_value = var;
  compiler_session->add(name, this->shared_from_this());
  return _llvm_value;
}

std::string ASTVarDecl::get_name() const {
  auto n = ast_cast<ASTIdentifier>(_children[0]);
  return n->get_name();
}

std::string ASTVarDecl::get_type_name() const {
  auto t = ast_cast<ASTTy>(_children[1]);
  return t->get_type_name();
}

llvm::Type *ASTVarDecl::to_llvm_type(CompilerSession *compiler_session) const {
  auto t = ast_cast<ASTTy>(_children[1]);
  return t->to_llvm_type(compiler_session);
}

llvm::Value *ASTVarDecl::get_llvm_value(CompilerSession *) const {
  return _llvm_value;
}

Value *ASTNumberLiteral::codegen(CompilerSession *compiler_session) {
  if (_is_float) {
    _llvm_value = ConstantFP::get(*compiler_session->get_context(), APFloat(_fvalue));
  } else {
    _llvm_value = ConstantInt::get(*compiler_session->get_context(), APInt(32, static_cast<uint64_t>(_ivalue), true));
  }
  return _llvm_value;
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
  if (ltype->isPointerTy()) {
    lhs = compiler_session->get_builder()->CreateLoad(lhs);
    ltype = lhs->getType();
  }
  if (rtype->isPointerTy()) {
    rhs = compiler_session->get_builder()->CreateLoad(rhs);
    rtype = rhs->getType();
  }

  Type *float_type = compiler_session->get_builder()->getFloatTy();
  if (!ltype->isIntegerTy() || !rtype->isIntegerTy()) {
    if (ltype->isIntegerTy()) {
      lhs = compiler_session->get_builder()->CreateSIToFP(lhs, float_type);
    }
    if (rtype->isIntegerTy()) {
      rhs = compiler_session->get_builder()->CreateSIToFP(rhs, float_type);
    }

    if (_type == ASTType::GT) {
      return compiler_session->get_builder()->CreateFCmpOGT(lhs, rhs, "gt");
    } else if (_type == ASTType::GE) {
      return compiler_session->get_builder()->CreateFCmpOGE(lhs, rhs, "ge");
    } else if (_type == ASTType::LT) {
      return compiler_session->get_builder()->CreateFCmpOLT(lhs, rhs, "lt");
    } else if (_type == ASTType::LE) {
      return compiler_session->get_builder()->CreateFCmpOLE(lhs, rhs, "le");
    }
  }
  if (_type == ASTType::GT) {
    return compiler_session->get_builder()->CreateICmpUGT(lhs, rhs, "gt");
  } else if (_type == ASTType::GE) {
    return compiler_session->get_builder()->CreateICmpUGE(lhs, rhs, "ge");
  } else if (_type == ASTType::LT) {
    return compiler_session->get_builder()->CreateICmpULT(lhs, rhs, "lt");
  } else if (_type == ASTType::LE) {
    return compiler_session->get_builder()->CreateICmpULE(lhs, rhs, "le");
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
  Value *to = _children[0]->codegen(compiler_session);
  if (!to) {
    report_code_error(_children[0]->_token, "Invalid left-hand operand of the assignment");
  }
  // codegen the rhs
  auto rhs = _children[1];
  Value *from = rhs->codegen(compiler_session);
  if (from->getType()->isPointerTy()) {
    from = compiler_session->get_builder()->CreateLoad(from);
  }
  if (!from) {
    report_code_error(rhs->_token, "Invalid expression for right-hand operand of the assignment");
  }

  // TODO: check type
  // TODO: implicit type conversion
  compiler_session->get_builder()->CreateStore(from, to);
  return to;
}

Value *ASTArgDecl::codegen(CompilerSession *compiler_session) {
  UNUSED(compiler_session);
  assert(false);
}

ASTNumberLiteral::ASTNumberLiteral(const std::string &str, bool is_float, Token *token, size_t token_index)
    : ASTLiteral(ASTType::NUM_LITERAL, op_precedence[ASTType::NUM_LITERAL], 0, token, token_index
) {
  _is_float = is_float;
  if (is_float) {
    _fvalue = std::stof(str);
  } else {
    _ivalue = std::stoi(str);
  }
}

llvm::Value *ASTNumberLiteral::get_llvm_value(CompilerSession *) const { return _llvm_value; }

std::string ASTNumberLiteral::get_type_name() const {
  // TODO: other type names
  if (_is_float) {
    return "float";
  } else {
    return "int";
  }
}

llvm::Type *ASTNumberLiteral::to_llvm_type(CompilerSession *compiler_session) const {
  // TODO: other types
  if (_is_float) {
    return compiler_session->get_builder()->getFloatTy();
  } else {
    return compiler_session->get_builder()->getInt32Ty();
  }
}

Ty ASTNumberLiteral::get_ty() const {
  // TODO: other types
  if (_is_float) {
    return Ty::FLOAT;
  } else {
    return TY_OR(Ty::INT, Ty::BIT32);
  }
}

std::string ASTNumberLiteral::to_string(bool print_prefix) const {
  std::string ret = "";
  if (print_prefix) {
    ret += ASTLiteral::to_string(print_prefix) + " ";
  }
  if (_is_float) { ret += std::to_string(_fvalue); } else { ret += std::to_string(_ivalue); }
  return ret;
}

ASTStringLiteral::ASTStringLiteral(Token *token, size_t token_index) : ASTLiteral(ASTType::STRING_LITERAL,
                                                                                  op_precedence[ASTType::STRING_LITERAL],
                                                                                  0,
                                                                                  token,
                                                                                  token_index
) {
  _svalue = token->value;
}

ASTAssignment::ASTAssignment(Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  _type = ASTType::ASSIGN;
  _lbp = op_precedence[_type];
}

llvm::Value *ASTStringLiteral::get_llvm_value(CompilerSession *) const {
  return _llvm_value;
}

Value *ASTStringLiteral::codegen(CompilerSession *compiler_session) {
  _llvm_type = compiler_session->get_builder()->getInt8PtrTy(); /// str as char*
  _llvm_value = compiler_session->get_builder()->CreateGlobalStringPtr(_svalue);
  return _llvm_value;
}

std::string ASTStringLiteral::get_type_name() const {
  return "str";
}

llvm::Type *ASTStringLiteral::to_llvm_type(CompilerSession *) const { return _llvm_type; }

Ty ASTStringLiteral::get_ty() const { return Ty::STRING; }

} // namespace tanlang
