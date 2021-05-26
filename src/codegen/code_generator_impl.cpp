#include "src/llvm_include.h"
#include "code_generator_impl.h"
#include "src/ast/ast_node.h"
#include "src/ast/ast_ty.h"
#include "src/ast/ast_func.h"
#include "compiler_session.h"
#include "src/ast/ast_member_access.h"
#include "intrinsic.h"
#include "src/common.h"
#include "src/analysis/type_system.h"

using namespace tanlang;

CodeGeneratorImpl::CodeGeneratorImpl(CompilerSession *cs) : _cs(cs), _h(ASTHelper(cs)) {}

Value *CodeGeneratorImpl::codegen(const ASTNodePtr &p) {
  if (p->_llvm_value) {
    return p->_llvm_value;
  }

  Value *ret = nullptr;
  switch (p->get_node_type()) {
    case ASTType::PROGRAM:
    case ASTType::STATEMENT:
      for (const auto &e : p->get_children()) {
        codegen(ast_must_cast<ASTNode>(e));
      }
      ret = nullptr;
      break;
      ///////////////////////// ops ///////////////////////////
    case ASTType::SUM:
    case ASTType::SUBTRACT:
    case ASTType::MULTIPLY:
    case ASTType::DIVIDE:
    case ASTType::MOD:
      ret = codegen_arithmetic(p);
      break;
    case ASTType::GT:
    case ASTType::GE:
    case ASTType::LT:
    case ASTType::LE:
    case ASTType::EQ:
    case ASTType::NE:
      ret = codegen_comparison(p);
      break;
    case ASTType::ASSIGN:
      ret = codegen_assignment(p);
      break;
    case ASTType::CAST:
      ret = codegen_cast(p);
      break;
    case ASTType::ADDRESS_OF:
      ret = codegen_address_of(p);
      break;
    case ASTType::RET:
      ret = codegen_return(p);
      break;
    case ASTType::LNOT:
      ret = codegen_lnot(p);
      break;
    case ASTType::BNOT:
      ret = codegen_bnot(p);
      break;
    case ASTType::IMPORT:
      ret = codegen_import(p);
      break;
    case ASTType::MEMBER_ACCESS: {
      auto pma = ast_cast<ASTMemberAccess>(p);
      TAN_ASSERT(pma);
      ret = codegen_member_access(pma);
      break;
    }
      ////////////////////////////// literals ////////////////////////
    case ASTType::ARRAY_LITERAL:
    case ASTType::NUM_LITERAL:
    case ASTType::CHAR_LITERAL:
    case ASTType::STRING_LITERAL: {
      ret = codegen_literals(p);
      break;
    }
      ///////////////////////////// other ////////////////////////////
    case ASTType::INTRINSIC: {
      auto pi = ast_cast<Intrinsic>(p);
      TAN_ASSERT(pi);
      ret = codegen_intrinsic(pi);
      break;
    }
    case ASTType::FUNC_DECL: {
      auto pf = ast_cast<ASTFunction>(p);
      TAN_ASSERT(pf);
      ret = codegen_func_decl(pf);
      break;
    }
    case ASTType::FUNC_CALL:
      ret = codegen_func_call(p);
      break;
    case ASTType::IF:
      ret = codegen_if(p);
      break;
    case ASTType::CONTINUE:
    case ASTType::BREAK:
      ret = codegen_break_continue(p);
      break;
    case ASTType::LOOP:
      ret = codegen_loop(p);
      break;
    case ASTType::VAR_DECL:
    case ASTType::ARG_DECL:
      ret = codegen_var_arg_decl(p);
      break;
    case ASTType::TY:
      // FIXME:
      //  ret = codegen_ty(ast_cast<ASTTy>(p));
      break;
    case ASTType::PARENTHESIS:
      ret = codegen_parenthesis(p);
      break;

      /////////////////// trivial codegen /////////////////
    case ASTType::ELSE:
      set_current_debug_location(p);
      // fallthrough
    case ASTType::ID:
      ret = codegen(p->get_child_at<ASTNode>(0));
      break;
    default:
      break;
  }
  p->_llvm_value = ret;
  return ret;
}

void CodeGeneratorImpl::set_current_debug_location(ASTNodePtr p) {
  _cs->set_current_debug_location(p->get_line(), p->get_col());
}

Value *CodeGeneratorImpl::codegen_arithmetic(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);
  /// unary plus/minus
  if (p->get_children_size() == 1) {
    if (!is_ast_type_in(p->get_node_type(), {ASTType::SUM, ASTType::SUBTRACT})) {
      report_error(p, "Invalid unary operation");
    }
    if (p->get_node_type() == ASTType::SUM) { return codegen(p->get_child_at<ASTNode>(0)); }
    else {
      auto *r = codegen(p->get_child_at<ASTNode>(0));
      if (p->get_child_at<ASTNode>(0)->_ty->_is_lvalue) { r = builder->CreateLoad(r); }
      if (r->getType()->isFloatingPointTy()) { return builder->CreateFNeg(r); }
      return builder->CreateNeg(r);
    }
  }

  /// binary operator
  auto lhs = p->get_child_at<ASTNode>(0);
  auto rhs = p->get_child_at<ASTNode>(1);
  Value *l = codegen(p->get_child_at<ASTNode>(0));
  Value *r = codegen(p->get_child_at<ASTNode>(1));
  TAN_ASSERT(l && r);
  TAN_ASSERT(p->get_children_size() > p->_dominant_idx);

  if (p->_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(_cs, r, rhs->_ty, lhs->_ty);
    l = TypeSystem::ConvertTo(_cs, l, lhs->_ty, lhs->_ty);
  } else {
    l = TypeSystem::ConvertTo(_cs, l, lhs->_ty, rhs->_ty);
    r = TypeSystem::ConvertTo(_cs, r, rhs->_ty, rhs->_ty);
  }

  if (l->getType()->isFloatingPointTy()) {
    /// float arithmetic
    if (p->get_node_type() == ASTType::MULTIPLY) {
      p->_llvm_value = builder->CreateFMul(l, r, "mul_tmp");
    } else if (p->get_node_type() == ASTType::DIVIDE) {
      p->_llvm_value = builder->CreateFDiv(l, r, "div_tmp");
    } else if (p->get_node_type() == ASTType::SUM) {
      p->_llvm_value = builder->CreateFAdd(l, r, "sum_tmp");
    } else if (p->get_node_type() == ASTType::SUBTRACT) {
      p->_llvm_value = builder->CreateFSub(l, r, "sub_tmp");
    } else if (p->get_node_type() == ASTType::MOD) {
      p->_llvm_value = builder->CreateFRem(l, r, "mod_tmp");
    } else { TAN_ASSERT(false); }
  } else {
    /// integer arithmetic
    if (p->get_node_type() == ASTType::MULTIPLY) {
      p->_llvm_value = builder->CreateMul(l, r, "mul_tmp");
    } else if (p->get_node_type() == ASTType::DIVIDE) {
      auto ty = p->get_child_at<ASTNode>(0)->_ty;
      if (ty->_is_unsigned) { p->_llvm_value = builder->CreateUDiv(l, r, "div_tmp"); }
      else { p->_llvm_value = builder->CreateSDiv(l, r, "div_tmp"); }
    } else if (p->get_node_type() == ASTType::SUM) {
      p->_llvm_value = builder->CreateAdd(l, r, "sum_tmp");
    } else if (p->get_node_type() == ASTType::SUBTRACT) {
      p->_llvm_value = builder->CreateSub(l, r, "sub_tmp");
    } else if (p->get_node_type() == ASTType::MOD) {
      auto ty = p->get_child_at<ASTNode>(0)->_ty;
      if (ty->_is_unsigned) { p->_llvm_value = builder->CreateURem(l, r, "mod_tmp"); }
      else { p->_llvm_value = builder->CreateSRem(l, r, "mod_tmp"); }
    } else { TAN_ASSERT(false); }
  }
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_bnot(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);
  auto *rhs = codegen(p->get_child_at<ASTNode>(0));
  if (!rhs) { report_error(p, "Invalid operand"); }
  if (p->get_child_at<ASTNode>(0)->_ty->_is_lvalue) {
    rhs = builder->CreateLoad(rhs);
  }
  return (p->_llvm_value = builder->CreateNot(rhs));
}

Value *CodeGeneratorImpl::codegen_lnot(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);
  auto *rhs = codegen(p->get_child_at<ASTNode>(0));
  if (!rhs) { report_error(p, "Invalid operand"); }
  if (p->get_child_at<ASTNode>(0)->_ty->_is_lvalue) {
    rhs = builder->CreateLoad(rhs);
  }
  /// get value size in bits
  auto size_in_bits = rhs->getType()->getPrimitiveSizeInBits();
  if (rhs->getType()->isFloatingPointTy()) {
    p->_llvm_value = builder->CreateFCmpOEQ(rhs, ConstantFP::get(builder->getFloatTy(), 0.0f));
  } else if (rhs->getType()->isSingleValueType()) {
    p->_llvm_value =
        builder->CreateICmpEQ(rhs, ConstantInt::get(builder->getIntNTy((unsigned) size_in_bits), 0, false));
  } else { report_error(p, "Invalid operand"); }
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_return(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);
  ASTNodePtr rhs = p->get_child_at<ASTNode>(0);
  auto *result = codegen(rhs);
  if (rhs->_ty->_is_lvalue) {
    result = builder->CreateLoad(result, "ret");
  }
  builder->CreateRet(result);
  return nullptr;
}

Value *CodeGeneratorImpl::codegen_comparison(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);
  auto lhs = p->get_child_at<ASTNode>(0);
  auto rhs = p->get_child_at<ASTNode>(1);
  Value *l = codegen(lhs);
  Value *r = codegen(rhs);
  TAN_ASSERT(l && r);
  TAN_ASSERT(p->get_children_size() > p->_dominant_idx);

  if (p->_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(_cs, r, rhs->_ty, lhs->_ty);
    l = TypeSystem::ConvertTo(_cs, l, lhs->_ty, lhs->_ty);
  } else {
    l = TypeSystem::ConvertTo(_cs, l, lhs->_ty, rhs->_ty);
    r = TypeSystem::ConvertTo(_cs, r, rhs->_ty, rhs->_ty);
  }

  if (l->getType()->isFloatingPointTy()) {
    if (p->get_node_type() == ASTType::EQ) {
      p->_llvm_value = builder->CreateFCmpOEQ(l, r, "eq");
    } else if (p->get_node_type() == ASTType::NE) {
      p->_llvm_value = builder->CreateFCmpONE(l, r, "ne");
    } else if (p->get_node_type() == ASTType::GT) {
      p->_llvm_value = builder->CreateFCmpOGT(l, r, "gt");
    } else if (p->get_node_type() == ASTType::GE) {
      p->_llvm_value = builder->CreateFCmpOGE(l, r, "ge");
    } else if (p->get_node_type() == ASTType::LT) {
      p->_llvm_value = builder->CreateFCmpOLT(l, r, "lt");
    } else if (p->get_node_type() == ASTType::LE) {
      p->_llvm_value = builder->CreateFCmpOLE(l, r, "le");
    }
  } else {
    if (p->get_node_type() == ASTType::EQ) {
      p->_llvm_value = builder->CreateICmpEQ(l, r, "eq");
    } else if (p->get_node_type() == ASTType::NE) {
      p->_llvm_value = builder->CreateICmpNE(l, r, "ne");
    } else if (p->get_node_type() == ASTType::GT) {
      p->_llvm_value = builder->CreateICmpUGT(l, r, "gt");
    } else if (p->get_node_type() == ASTType::GE) {
      p->_llvm_value = builder->CreateICmpUGE(l, r, "ge");
    } else if (p->get_node_type() == ASTType::LT) {
      p->_llvm_value = builder->CreateICmpULT(l, r, "lt");
    } else if (p->get_node_type() == ASTType::LE) {
      p->_llvm_value = builder->CreateICmpULE(l, r, "le");
    }
  }
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_assignment(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);
  /// _codegen the rhs
  auto lhs = p->get_child_at<ASTNode>(0);
  auto rhs = p->get_child_at<ASTNode>(1);
  Value *from = codegen(rhs);
  Value *to = codegen(lhs);
  if (!from) { report_error(lhs, "Invalid expression for right-hand operand of the assignment"); }
  if (!to) { report_error(rhs, "Invalid left-hand operand of the assignment"); }
  if (!lhs->_ty->_is_lvalue) { report_error(lhs, "Value can only be assigned to lvalue"); }

  from = TypeSystem::ConvertTo(_cs, from, rhs->_ty, lhs->_ty);
  builder->CreateStore(from, to);
  p->_llvm_value = to;
  return to;
}

Value *CodeGeneratorImpl::codegen_cast(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);
  auto lhs = p->get_child_at<ASTNode>(0);
  auto *dest_type = TypeSystem::ToLLVMType(_cs, p->get_child_at<ASTNode>(1)->_ty);
  Value *val = codegen(lhs);
  Value *ret = nullptr;
  val = TypeSystem::ConvertTo(_cs, val, lhs->_ty, p->get_child_at<ASTNode>(1)->_ty);
  if (lhs->_ty->_is_lvalue) {
    ret = create_block_alloca(builder->GetInsertBlock(), dest_type, 1, "casted");
    builder->CreateStore(val, ret);
  } else { ret = val; }
  p->_llvm_value = ret;
  return ret;
}

Value *CodeGeneratorImpl::codegen_var_arg_decl(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  if (!p->_ty->_resolved) {
    report_error(p, "Unknown type");
  }
  Type *type = TypeSystem::ToLLVMType(_cs, p->_ty);
  p->_llvm_value = create_block_alloca(builder->GetInsertBlock(), type, 1, p->get_data<str>());

  // FIXME: default value of var declaration
  // if (p->get_node_type() == ASTType::VAR_DECL) { /// don't do this for arg_decl
  //   auto *default_value = codegen_ty(p->_ty);
  //   if (default_value) { builder->CreateStore(default_value, codegen_ty(p->_ty)); }
  // }
  /// debug info
  {
    auto *di_builder = _cs->_di_builder;
    auto *curr_di_scope = _cs->get_current_di_scope();
    auto *arg_meta = TypeSystem::ToLLVMMeta(_cs, p->_ty);
    auto *di_arg = di_builder->createAutoVariable(curr_di_scope,
        p->get_data<str>(),
        _cs->get_di_file(),
        (unsigned) p->get_line(),
        (DIType *) arg_meta);
    di_builder->insertDeclare(p->_llvm_value,
        di_arg,
        _cs->_di_builder->createExpression(),
        llvm::DebugLoc::get((unsigned) p->get_line(), (unsigned) p->get_col(), curr_di_scope),
        builder->GetInsertBlock());
  }
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_address_of(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);
  auto *val = codegen(p->get_child_at<ASTNode>(0));
  if (p->get_child_at<ASTNode>(0)->_ty->_is_lvalue) { /// lvalue, the val itself is a pointer to real value
    p->_llvm_value = val;
  } else { /// rvalue, create an anonymous variable, and get address of it
    p->_llvm_value = create_block_alloca(builder->GetInsertBlock(), val->getType(), 1, "anonymous");
    builder->CreateStore(val, p->_llvm_value);
  }
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_parenthesis(const ASTNodePtr &p) {
  set_current_debug_location(p);
  // FIXME: multiple expressions in the parenthesis?
  p->_llvm_value = codegen(p->get_child_at<ASTNode>(0));
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_import(const ASTNodePtr &p) {
  set_current_debug_location(p);
  for (auto &n: p->get_children()) {
    auto f = ast_must_cast<ASTFunction>(n);
    /// do nothing for already defined intrinsics
    auto *func = _cs->get_module()->getFunction(f->get_data<str>());
    if (!func) {
      codegen_func_prototype(f);
    } else {
      f->_func = func;
    }
  }
  return nullptr;
}

Value *CodeGeneratorImpl::codegen_intrinsic(const IntrinsicPtr &p) {
  set_current_debug_location(p);

  Value *ret = nullptr;
  switch (p->_intrinsic_type) {
    /// trivial codegen
    case IntrinsicType::GET_DECL:
    case IntrinsicType::LINENO:
    case IntrinsicType::NOOP:
    case IntrinsicType::ABORT:
    case IntrinsicType::FILENAME: {
      ret = codegen(p->get_child_at<ASTNode>(0));
      break;
    }
      /// others
    case IntrinsicType::STACK_TRACE:
      // TODO: codegen of stack trace
      break;
    default:
      break;
  }
  return ret;
}

Value *CodeGeneratorImpl::codegen_ty(const ASTTyPtr &p) {
  auto *builder = _cs->_builder;
  TAN_ASSERT(p->_resolved);
  Ty base = TY_GET_BASE(p->_tyty);
  Value *ret = nullptr;
  Type *type = TypeSystem::ToLLVMType(_cs, p);
  switch (base) {
    case Ty::INT:
    case Ty::CHAR:
    case Ty::BOOL:
    case Ty::ENUM:
      ret = ConstantInt::get(type, std::get<uint64_t>(p->_default_value));
      break;
    case Ty::FLOAT:
      ret = ConstantFP::get(type, std::get<float>(p->_default_value));
      break;
    case Ty::DOUBLE:
      ret = ConstantFP::get(type, std::get<double>(p->_default_value));
      break;
    case Ty::STRING:
      ret = builder->CreateGlobalStringPtr(std::get<str>(p->_default_value));
      break;
    case Ty::VOID:
      TAN_ASSERT(false);
    case Ty::STRUCT: {
      vector<llvm::Constant *> values{};
      size_t n = p->get_children_size();
      for (size_t i = 1; i < n; ++i) {
        values.push_back((llvm::Constant *) codegen_ty(p->get_child_at<ASTNode>(i)->_ty));
      }
      ret = ConstantStruct::get((StructType *) TypeSystem::ToLLVMType(_cs, p), values);
      break;
    }
    case Ty::POINTER:
      ret = ConstantPointerNull::get((PointerType *) type);
      break;
    case Ty::ARRAY: {
      auto *e_type = TypeSystem::ToLLVMType(_cs, p->get_child_at<ASTNode>(0)->_ty);
      size_t n = p->get_children_size();
      ret = create_block_alloca(builder->GetInsertBlock(), e_type, n, "const_array");
      for (size_t i = 0; i < n; ++i) {
        auto *idx = builder->getInt32((unsigned) i);
        auto *e_val = codegen_ty(p->get_child_at<ASTNode>(i)->_ty);
        auto *e_ptr = builder->CreateGEP(ret, idx);
        builder->CreateStore(e_val, e_ptr);
      }
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return ret;
}

Value *CodeGeneratorImpl::codegen_literals(const ASTNodePtr &p) {
  set_current_debug_location(p);
  Type *type = TypeSystem::ToLLVMType(_cs, p->_ty);
  Value *ret = nullptr;
  switch (p->_ty->_tyty) {
    case Ty::INT:
    case Ty::CHAR:
    case Ty::BOOL:
    case Ty::ENUM:
      ret = ConstantInt::get(type, p->get_data<uint64_t>());
      break;
    case Ty::STRING:
      ret = _cs->_builder->CreateGlobalStringPtr(p->get_data<str>());
      break;
    case Ty::FLOAT:
      ret = ConstantFP::get(type, p->get_data<double>());
      break;
    case Ty::DOUBLE:
      ret = ConstantFP::get(type, p->get_data<double>());
      break;
    default:
      TAN_ASSERT(false);
  }
  return ret;
}

// FIXME: write an ASTNodeError class
void CodeGeneratorImpl::report_error(const ParsableASTNodePtr &p, const str &message) {
  ::report_error(_cs->_filename, p->get_token(), message);
}
