#include "code_generator_impl.h"
#include "src/ast/ast_type.h"
#include "src/ast/intrinsic.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"
#include "src/analysis/type_system.h"
#include "compiler_session.h"
#include "src/common.h"
#include "src/llvm_include.h"

using namespace tanlang;

CodeGeneratorImpl::CodeGeneratorImpl(CompilerSession *cs) : _cs(cs), _h(ASTHelper(cs)) {}

Value *CodeGeneratorImpl::codegen(const ASTBasePtr &p) {
  if (p->_llvm_value) {
    return p->_llvm_value;
  }

  Value *ret = nullptr;
  switch (p->get_node_type()) {
    case ASTNodeType::PROGRAM:
    case ASTNodeType::STATEMENT:
      ret = codegen_stmt(p);
      break;
    case ASTNodeType::BOP:
      codegen_bop(p);
      break;
    case ASTNodeType::UOP:
      codegen_uop(p);
      break;
    case ASTNodeType::RET:
      ret = codegen_return(p);
      break;
    case ASTNodeType::IMPORT:
      ret = codegen_import(p);
      break;
      ////////////////////////////// literals ////////////////////////
    case ASTNodeType::ARRAY_LITERAL:
    case ASTNodeType::INTEGER_LITERAL:
    case ASTNodeType::FLOAT_LITERAL:
    case ASTNodeType::CHAR_LITERAL:
    case ASTNodeType::STRING_LITERAL: {
      ret = codegen_literals(p);
      break;
    }
      ///////////////////////////// other ////////////////////////////
    case ASTNodeType::INTRINSIC: {
      auto pi = ast_cast<Intrinsic>(p);
      TAN_ASSERT(pi);
      ret = codegen_intrinsic(pi);
      break;
    }
    case ASTNodeType::FUNC_DECL:
      ret = codegen_func_decl(ast_must_cast<FunctionDecl>(p));
      break;
    case ASTNodeType::FUNC_CALL:
      ret = codegen_func_call(p);
      break;
    case ASTNodeType::IF:
      ret = codegen_if(p);
      break;
    case ASTNodeType::CONTINUE:
    case ASTNodeType::BREAK:
      ret = codegen_break_continue(p);
      break;
    case ASTNodeType::LOOP:
      ret = codegen_loop(p);
      break;
    case ASTNodeType::VAR_DECL:
    case ASTNodeType::ARG_DECL:
      ret = codegen_var_arg_decl(p);
      break;
    case ASTNodeType::TY:
      // FIXME:
      //  ret = codegen_ty(ast_cast<ASTType>(p));
      break;
    case ASTNodeType::PARENTHESIS:
      ret = codegen_parenthesis(p);
      break;
    case ASTNodeType::ID:
      ret = codegen_identifier(p);
      break;
    default:
      break;
  }
  p->_llvm_value = ret;
  return ret;
}

void CodeGeneratorImpl::set_current_debug_location(ASTBasePtr p) {
  _cs->set_current_debug_location(p->get_line(), p->get_col());
}

Value *CodeGeneratorImpl::codegen_bnot(const ASTBasePtr &_p) {
  auto p = ast_must_cast<UnaryOperator>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto *rhs = codegen(p->get_rhs());
  if (!rhs) {
    report_error(p, "Invalid operand");
  }
  if (p->get_rhs()->get_type()->_is_lvalue) {
    rhs = builder->CreateLoad(rhs);
  }
  return p->_llvm_value = builder->CreateNot(rhs);
}

Value *CodeGeneratorImpl::codegen_lnot(const ASTBasePtr &_p) {
  auto p = ast_must_cast<UnaryOperator>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto *rhs = codegen(p->get_rhs());

  if (!rhs) {
    report_error(p, "Invalid operand");
  }

  if (p->get_rhs()->get_type()->_is_lvalue) {
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

Value *CodeGeneratorImpl::codegen_return(const ASTBasePtr &_p) {
  auto p = ast_must_cast<Return>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto rhs = p->get_rhs();
  auto *result = codegen(rhs);
  if (rhs->get_type()->_is_lvalue) {
    result = builder->CreateLoad(result, "ret");
  }
  builder->CreateRet(result);
  return nullptr;
}

Value *CodeGeneratorImpl::codegen_var_arg_decl(const ASTBasePtr &_p) {
  auto p = ast_must_cast<Decl>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  if (!p->get_type()->_resolved) {
    report_error(p, "Unknown type");
  }
  Type *type = TypeSystem::ToLLVMType(_cs, p->get_type());
  p->_llvm_value = create_block_alloca(builder->GetInsertBlock(), type, 1, p->get_name());

  // FIXME: default value of var declaration
  // if (p->get_node_type() == ASTNodeType::VAR_DECL) { /// don't do this for arg_decl
  //   auto *default_value = codegen_ty(p->_type);
  //   if (default_value) { builder->CreateStore(default_value, codegen_ty(p->_type)); }
  // }
  /// debug info
  {
    auto *di_builder = _cs->_di_builder;
    auto *curr_di_scope = _cs->get_current_di_scope();
    auto *arg_meta = TypeSystem::ToLLVMMeta(_cs, p->get_type());
    auto *di_arg = di_builder->createAutoVariable(curr_di_scope,
        p->get_name(),
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

Value *CodeGeneratorImpl::codegen_address_of(const ASTBasePtr &_p) {
  auto p = ast_must_cast<UnaryOperator>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto *val = codegen(p->get_rhs());
  if (p->get_rhs()->get_type()->_is_lvalue) { /// lvalue, the val itself is a pointer to real value
    p->_llvm_value = val;
  } else { /// rvalue, create an anonymous variable, and get address of it
    p->_llvm_value = create_block_alloca(builder->GetInsertBlock(), val->getType(), 1, "anonymous");
    builder->CreateStore(val, p->_llvm_value);
  }
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_parenthesis(const ASTBasePtr &_p) {
  auto p = ast_must_cast<Parenthesis>(_p);

  set_current_debug_location(p);

  return p->_llvm_value = codegen(p->get_sub());
}

Value *CodeGeneratorImpl::codegen_import(const ASTBasePtr &_p) {
  auto p = ast_must_cast<Import>(_p);

  set_current_debug_location(p);
  for (const FunctionDeclPtr &f: p->get_imported_funcs()) {
    /// do nothing for already defined intrinsics
    auto *func = _cs->get_module()->getFunction(f->get_name());
    if (!func) {
      codegen_func_prototype(f);
    } else {
      f->_llvm_value = func;
    }
  }
  return nullptr;
}

Value *CodeGeneratorImpl::codegen_intrinsic(const IntrinsicPtr &p) {
  set_current_debug_location(p);

  Value *ret = nullptr;
  switch (p->get_intrinsic_type()) {
    /// trivial codegen
    case IntrinsicType::GET_DECL:
    case IntrinsicType::LINENO:
    case IntrinsicType::NOOP:
    case IntrinsicType::ABORT:
    case IntrinsicType::FILENAME: {
      ret = codegen(p->get_sub());
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

Value *CodeGeneratorImpl::codegen_ty(const ASTTypePtr &p) {
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
      size_t n = p->_sub_types.size();
      for (size_t i = 1; i < n; ++i) {
        values.push_back((llvm::Constant *) codegen(p->_sub_types[i]));
      }
      ret = ConstantStruct::get((StructType *) TypeSystem::ToLLVMType(_cs, p), values);
      break;
    }
    case Ty::POINTER:
      ret = ConstantPointerNull::get((PointerType *) type);
      break;
    case Ty::ARRAY: {
      auto *e_type = TypeSystem::ToLLVMType(_cs, p->_sub_types[0]);
      size_t n = p->_sub_types.size();
      ret = create_block_alloca(builder->GetInsertBlock(), e_type, n, "const_array");
      for (size_t i = 0; i < n; ++i) {
        auto *idx = builder->getInt32((unsigned) i);
        auto *e_val = codegen_ty(p->_sub_types[i]);
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

// TODO: merge some of the code with codegen_ty()
Value *CodeGeneratorImpl::codegen_literals(const ASTBasePtr &_p) {
  auto p = ast_must_cast<Literal>(_p);

  set_current_debug_location(p);
  auto *builder = _cs->_builder;

  Type *type = TypeSystem::ToLLVMType(_cs, p->get_type());
  Value *ret = nullptr;
  Ty t = TY_GET_BASE(p->get_type()->_tyty);
  switch (t) {
    case Ty::CHAR:
      ret = ConstantInt::get(type, ast_must_cast<CharLiteral>(p)->get_value());
      break;
    case Ty::INT:
    case Ty::BOOL:
    case Ty::ENUM:
      ret = ConstantInt::get(type, ast_must_cast<IntegerLiteral>(p)->get_value());
      break;
    case Ty::STRING:
      ret = builder->CreateGlobalStringPtr(ast_must_cast<StringLiteral>(p)->get_value());
      break;
    case Ty::FLOAT:
    case Ty::DOUBLE:
      ret = ConstantFP::get(type, ast_must_cast<FloatLiteral>(p)->get_value());
      break;
    case Ty::ARRAY: {
      auto arr = ast_must_cast<ArrayLiteral>(p);

      /// element type
      auto elements = arr->get_elements();
      auto *e_type = TypeSystem::ToLLVMType(_cs, elements[0]->get_type());

      /// codegen element values
      size_t n = elements.size();
      ret = create_block_alloca(builder->GetInsertBlock(), e_type, n, "const_array");
      for (size_t i = 0; i < n; ++i) {
        auto *idx = builder->getInt32((unsigned) i);
        auto *e_val = codegen(elements[i]);
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

// FIXME: write an ASTBaseError class
void CodeGeneratorImpl::report_error(const ASTBasePtr &p, const str &message) {
  ::report_error(_cs->_filename, p->get_token(), message);
}

Value *CodeGeneratorImpl::codegen_stmt(const ASTBasePtr &_p) {
  auto p = ast_must_cast<CompoundStmt>(_p);

  for (const auto &e : p->get_children()) {
    codegen(e);
  }
  return nullptr;
}

Value *CodeGeneratorImpl::codegen_uop(const ASTBasePtr &_p) {
  auto p = ast_must_cast<UnaryOperator>(_p);
  Value *ret = nullptr;

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto rhs = p->get_rhs();
  switch (p->get_op()) {
    case UnaryOpKind::LNOT:
      ret = codegen_lnot(p);
      break;
    case UnaryOpKind::BNOT:
      ret = codegen_bnot(p);
      break;
    case UnaryOpKind::ADDRESS_OF:
      ret = codegen_address_of(p);
      break;
    case UnaryOpKind::PLUS:
      ret = codegen(rhs);
      break;
    case UnaryOpKind::MINUS: {
      auto *r = codegen(rhs);
      if (rhs->get_type()->_is_lvalue) {
        r = builder->CreateLoad(r);
      }
      if (r->getType()->isFloatingPointTy()) {
        ret = builder->CreateFNeg(r);
      } else {
        ret = builder->CreateNeg(r);
      }
      break;
    }
    default:
      TAN_ASSERT(false);
      break;
  }
  return ret;
}

Value *CodeGeneratorImpl::codegen_bop(const ASTBasePtr &_p) {
  auto p = ast_must_cast<BinaryOperator>(_p);
  Value *ret = nullptr;

  switch (p->get_op()) {
    case BinaryOpKind::ASSIGN:
      ret = codegen_assignment(p);
      break;
    case BinaryOpKind::SUM:
    case BinaryOpKind::SUBTRACT:
    case BinaryOpKind::MULTIPLY:
    case BinaryOpKind::DIVIDE:
    case BinaryOpKind::MOD:
      ret = codegen_arithmetic(p);
      break;
    case BinaryOpKind::BAND:
    case BinaryOpKind::LAND:
    case BinaryOpKind::BOR:
    case BinaryOpKind::LOR:
    case BinaryOpKind::XOR:
      // TODO: implement codegen of the above operators
      TAN_ASSERT(false);
      break;
    case BinaryOpKind::GT:
    case BinaryOpKind::GE:
    case BinaryOpKind::LT:
    case BinaryOpKind::LE:
    case BinaryOpKind::EQ:
    case BinaryOpKind::NE:
      ret = codegen_comparison(p);
      break;
    case BinaryOpKind::CAST:
      ret = codegen_cast(p);
      break;
    case BinaryOpKind::MEMBER_ACCESS:
      ret = codegen_member_access(ast_must_cast<MemberAccess>(p));
      break;
    default:
      TAN_ASSERT(false);
      break;
  }

  return ret;
}

Value *CodeGeneratorImpl::codegen_assignment(const ASTBasePtr &_p) {
  auto p = ast_must_cast<BinaryOperator>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  /// _codegen the lhs and rhs
  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();
  Value *from = codegen(rhs);
  Value *to = codegen(lhs);
  if (!from) { report_error(lhs, "Invalid expression for right-hand operand of the assignment"); }
  if (!to) { report_error(rhs, "Invalid left-hand operand of the assignment"); }
  if (!lhs->get_type()->_is_lvalue) { report_error(lhs, "Value can only be assigned to lvalue"); }

  from = TypeSystem::ConvertTo(_cs, from, rhs->get_type(), lhs->get_type());
  builder->CreateStore(from, to);
  p->_llvm_value = to;
  return to;
}

Value *CodeGeneratorImpl::codegen_arithmetic(const ASTBasePtr &_p) {
  auto p = ast_must_cast<BinaryOperator>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  /// binary operator
  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();
  Value *l = codegen(lhs);
  Value *r = codegen(rhs);
  if (!l) { report_error(lhs, "Invalid expression for right-hand operand"); }
  if (!r) { report_error(rhs, "Invalid expression for left-hand operand"); }

  if (p->_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(_cs, r, rhs->get_type(), lhs->get_type());
    l = TypeSystem::ConvertTo(_cs, l, lhs->get_type(), lhs->get_type());
  } else {
    l = TypeSystem::ConvertTo(_cs, l, lhs->get_type(), rhs->get_type());
    r = TypeSystem::ConvertTo(_cs, r, rhs->get_type(), rhs->get_type());
  }

  switch (p->get_op()) {
    case BinaryOpKind::MULTIPLY:
      break;
    case BinaryOpKind::DIVIDE:
      break;
    case BinaryOpKind::SUM:
      break;
    case BinaryOpKind::SUBTRACT:
      break;
    case BinaryOpKind::MOD:
      break;
    default:
      TAN_ASSERT(false);
      break;
  }

  if (l->getType()->isFloatingPointTy()) {
    /// float arithmetic
    switch (p->get_op()) {
      case BinaryOpKind::MULTIPLY:
        p->_llvm_value = builder->CreateFMul(l, r, "mul_tmp");
        break;
      case BinaryOpKind::DIVIDE:
        p->_llvm_value = builder->CreateFDiv(l, r, "div_tmp");
        break;
      case BinaryOpKind::SUM:
        p->_llvm_value = builder->CreateFAdd(l, r, "sum_tmp");
        break;
      case BinaryOpKind::SUBTRACT:
        p->_llvm_value = builder->CreateFSub(l, r, "sub_tmp");
        break;
      case BinaryOpKind::MOD:
        p->_llvm_value = builder->CreateFRem(l, r, "mod_tmp");
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
  } else {
    /// integer arithmetic
    switch (p->get_op()) {
      case BinaryOpKind::MULTIPLY:
        p->_llvm_value = builder->CreateMul(l, r, "mul_tmp");
        break;
      case BinaryOpKind::DIVIDE: {
        auto ty = lhs->get_type();
        if (ty->_is_unsigned) {
          p->_llvm_value = builder->CreateUDiv(l, r, "div_tmp");
        } else {
          p->_llvm_value = builder->CreateSDiv(l, r, "div_tmp");
        }
        break;
      }
      case BinaryOpKind::SUM:
        p->_llvm_value = builder->CreateAdd(l, r, "sum_tmp");
        break;
      case BinaryOpKind::SUBTRACT:
        p->_llvm_value = builder->CreateSub(l, r, "sub_tmp");
        break;
      case BinaryOpKind::MOD: {
        auto ty = lhs->get_type();
        if (ty->_is_unsigned) {
          p->_llvm_value = builder->CreateURem(l, r, "mod_tmp");
        } else {
          p->_llvm_value = builder->CreateSRem(l, r, "mod_tmp");
        }
        break;
      }
      default:
        TAN_ASSERT(false);
        break;
    }
  }
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_comparison(const ASTBasePtr &_p) {
  auto p = ast_must_cast<BinaryOperator>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();
  Value *l = codegen(lhs);
  Value *r = codegen(rhs);
  if (!l) { report_error(lhs, "Invalid expression for right-hand operand"); }
  if (!r) { report_error(rhs, "Invalid expression for left-hand operand"); }

  if (p->_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(_cs, r, rhs->get_type(), lhs->get_type());
    l = TypeSystem::ConvertTo(_cs, l, lhs->get_type(), lhs->get_type());
  } else {
    l = TypeSystem::ConvertTo(_cs, l, lhs->get_type(), rhs->get_type());
    r = TypeSystem::ConvertTo(_cs, r, rhs->get_type(), rhs->get_type());
  }

  if (l->getType()->isFloatingPointTy()) {
    switch (p->get_op()) {
      case BinaryOpKind::EQ:
        p->_llvm_value = builder->CreateFCmpOEQ(l, r, "eq");
        break;
      case BinaryOpKind::NE:
        p->_llvm_value = builder->CreateFCmpONE(l, r, "ne");
        break;
      case BinaryOpKind::GT:
        p->_llvm_value = builder->CreateFCmpOGT(l, r, "gt");
        break;
      case BinaryOpKind::GE:
        p->_llvm_value = builder->CreateFCmpOGE(l, r, "ge");
        break;
      case BinaryOpKind::LT:
        p->_llvm_value = builder->CreateFCmpOLT(l, r, "lt");
        break;
      case BinaryOpKind::LE:
        p->_llvm_value = builder->CreateFCmpOLE(l, r, "le");
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
  } else {
    switch (p->get_op()) {
      case BinaryOpKind::EQ:
        p->_llvm_value = builder->CreateICmpEQ(l, r, "eq");
        break;
      case BinaryOpKind::NE:
        p->_llvm_value = builder->CreateICmpNE(l, r, "ne");
        break;
      case BinaryOpKind::GT:
        p->_llvm_value = builder->CreateICmpUGT(l, r, "gt");
        break;
      case BinaryOpKind::GE:
        p->_llvm_value = builder->CreateICmpUGE(l, r, "ge");
        break;
      case BinaryOpKind::LT:
        p->_llvm_value = builder->CreateICmpULT(l, r, "lt");
        break;
      case BinaryOpKind::LE:
        p->_llvm_value = builder->CreateICmpULE(l, r, "le");
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
  }
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_cast(const ASTBasePtr &_p) {
  auto p = ast_must_cast<BinaryOperator>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();
  auto *dest_type = TypeSystem::ToLLVMType(_cs, rhs->get_type());

  Value *val = codegen(lhs);
  if (!val) { report_error(lhs, "Invalid expression for left-hand operand"); }

  Value *ret = nullptr;
  val = TypeSystem::ConvertTo(_cs, val, lhs->get_type(), rhs->get_type());

  if (lhs->get_type()->_is_lvalue) {
    ret = create_block_alloca(builder->GetInsertBlock(), dest_type, 1, "casted");
    builder->CreateStore(val, ret);
  } else {
    ret = val;
  }

  return p->_llvm_value = ret;
}

Value *CodeGeneratorImpl::codegen_identifier(const ASTBasePtr &_p) {
  auto p = ast_must_cast<Identifier>(_p);
  return p->_llvm_value = codegen(p->_referred);
}
