#include "src/ast/ast_control_flow.h"
#include "src/ast/ast_func.h"
#include "src/analysis/analysis.h"
#include "src/codegen/codegen.h"
#include "compiler_session.h"
#include "src/llvm_include.h"
#include "src/ast/ast_ty.h"
#include "src/analysis/type_system.h"
#include "src/common.h"
#include "token.h"

namespace tanlang {

static Value *codegen_arithmetic(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  /// unary plus/minus
  if (p->_children.size() == 1) {
    if (!is_ast_type_in(p->_type, {ASTType::SUM, ASTType::SUBTRACT})) { error(cs, "Invalid unary operation"); }
    if (p->_type == ASTType::SUM) { return codegen(cs, p->_children[0]); }
    else {
      auto *r = codegen(cs, p->_children[0]);
      if (p->_children[0]->_ty->_is_lvalue) { r = builder->CreateLoad(r); }
      if (r->getType()->isFloatingPointTy()) { return builder->CreateFNeg(r); }
      return builder->CreateNeg(r);
    }
  }

  /// binary operator
  auto lhs = p->_children[0];
  auto rhs = p->_children[1];
  Value *l = codegen(cs, p->_children[0]);
  Value *r = codegen(cs, p->_children[1]);
  TAN_ASSERT(l && r);
  TAN_ASSERT(p->_children.size() > p->_dominant_idx);

  if (p->_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(cs, r, rhs->_ty, lhs->_ty);
    l = TypeSystem::ConvertTo(cs, l, lhs->_ty, lhs->_ty);
  } else {
    l = TypeSystem::ConvertTo(cs, l, lhs->_ty, rhs->_ty);
    r = TypeSystem::ConvertTo(cs, r, rhs->_ty, rhs->_ty);
  }

  if (l->getType()->isFloatingPointTy()) {
    /// float arithmetic
    if (p->_type == ASTType::MULTIPLY) {
      p->_llvm_value = builder->CreateFMul(l, r, "mul_tmp");
    } else if (p->_type == ASTType::DIVIDE) {
      p->_llvm_value = builder->CreateFDiv(l, r, "div_tmp");
    } else if (p->_type == ASTType::SUM) {
      p->_llvm_value = builder->CreateFAdd(l, r, "sum_tmp");
    } else if (p->_type == ASTType::SUBTRACT) {
      p->_llvm_value = builder->CreateFSub(l, r, "sub_tmp");
    } else if (p->_type == ASTType::MOD) {
      p->_llvm_value = builder->CreateFRem(l, r, "mod_tmp");
    } else { TAN_ASSERT(false); }
  } else {
    /// integer arithmetic
    if (p->_type == ASTType::MULTIPLY) {
      p->_llvm_value = builder->CreateMul(l, r, "mul_tmp");
    } else if (p->_type == ASTType::DIVIDE) {
      auto ty = p->_children[0]->_ty;
      if (ty->_is_unsigned) { p->_llvm_value = builder->CreateUDiv(l, r, "div_tmp"); }
      else { p->_llvm_value = builder->CreateSDiv(l, r, "div_tmp"); }
    } else if (p->_type == ASTType::SUM) {
      p->_llvm_value = builder->CreateAdd(l, r, "sum_tmp");
    } else if (p->_type == ASTType::SUBTRACT) {
      p->_llvm_value = builder->CreateSub(l, r, "sub_tmp");
    } else if (p->_type == ASTType::MOD) {
      auto ty = p->_children[0]->_ty;
      if (ty->_is_unsigned) { p->_llvm_value = builder->CreateURem(l, r, "mod_tmp"); }
      else { p->_llvm_value = builder->CreateSRem(l, r, "mod_tmp"); }
    } else { TAN_ASSERT(false); }
  }
  return p->_llvm_value;
}

static Value *codegen_lnot(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  auto *rhs = codegen(cs, p->_children[0]);
  if (!rhs) { error(cs, "Invalid operand"); }
  if (is_lvalue(p->_children[0])) { rhs = builder->CreateLoad(rhs); }
  /// get value size in bits
  auto size_in_bits = rhs->getType()->getPrimitiveSizeInBits();
  if (rhs->getType()->isFloatingPointTy()) {
    p->_llvm_value = builder->CreateFCmpOEQ(rhs, ConstantFP::get(builder->getFloatTy(), 0.0f));
  } else if (rhs->getType()->isSingleValueType()) {
    p->_llvm_value =
        builder->CreateICmpEQ(rhs, ConstantInt::get(builder->getIntNTy((unsigned) size_in_bits), 0, false));
  } else { error(cs, "Invalid operand"); }
  return p->_llvm_value;
}

static Value *codegen_bnot(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  auto *rhs = codegen(cs, p->_children[0]);
  if (!rhs) { error(cs, "Invalid operand"); }
  if (is_lvalue(p->_children[0])) { rhs = builder->CreateLoad(rhs); }
  return (p->_llvm_value = builder->CreateNot(rhs));
}

static Value *codegen_return(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  auto *result = codegen(cs, p->_children[0]);
  if (is_lvalue(p->_children[0])) { result = builder->CreateLoad(result, "ret"); }
  builder->CreateRet(result);
  return nullptr;
}

static Value *codegen_comparison(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  auto lhs = p->_children[0];
  auto rhs = p->_children[1];
  Value *l = codegen(cs, lhs);
  Value *r = codegen(cs, rhs);
  TAN_ASSERT(l && r);
  TAN_ASSERT(p->_children.size() > p->_dominant_idx);

  if (p->_dominant_idx == 0) {
    r = TypeSystem::ConvertTo(cs, r, rhs->_ty, lhs->_ty);
    l = TypeSystem::ConvertTo(cs, l, lhs->_ty, lhs->_ty);
  } else {
    l = TypeSystem::ConvertTo(cs, l, lhs->_ty, rhs->_ty);
    r = TypeSystem::ConvertTo(cs, r, rhs->_ty, rhs->_ty);
  }

  if (l->getType()->isFloatingPointTy()) {
    if (p->_type == ASTType::EQ) {
      p->_llvm_value = builder->CreateFCmpOEQ(l, r, "eq");
    } else if (p->_type == ASTType::NE) {
      p->_llvm_value = builder->CreateFCmpONE(l, r, "ne");
    } else if (p->_type == ASTType::GT) {
      p->_llvm_value = builder->CreateFCmpOGT(l, r, "gt");
    } else if (p->_type == ASTType::GE) {
      p->_llvm_value = builder->CreateFCmpOGE(l, r, "ge");
    } else if (p->_type == ASTType::LT) {
      p->_llvm_value = builder->CreateFCmpOLT(l, r, "lt");
    } else if (p->_type == ASTType::LE) {
      p->_llvm_value = builder->CreateFCmpOLE(l, r, "le");
    }
  } else {
    if (p->_type == ASTType::EQ) {
      p->_llvm_value = builder->CreateICmpEQ(l, r, "eq");
    } else if (p->_type == ASTType::NE) {
      p->_llvm_value = builder->CreateICmpNE(l, r, "ne");
    } else if (p->_type == ASTType::GT) {
      p->_llvm_value = builder->CreateICmpUGT(l, r, "gt");
    } else if (p->_type == ASTType::GE) {
      p->_llvm_value = builder->CreateICmpUGE(l, r, "ge");
    } else if (p->_type == ASTType::LT) {
      p->_llvm_value = builder->CreateICmpULT(l, r, "lt");
    } else if (p->_type == ASTType::LE) {
      p->_llvm_value = builder->CreateICmpULE(l, r, "le");
    }
  }
  return p->_llvm_value;
}

static Value *codegen_assignment(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  /// _codegen the rhs
  auto lhs = p->_children[0];
  auto rhs = p->_children[1];
  Value *from = codegen(cs, rhs);
  Value *to = codegen(cs, lhs);
  if (!from) { error(cs, "Invalid expression for right-hand operand of the assignment"); }
  if (!to) { error(cs, "Invalid left-hand operand of the assignment"); }
  if (!lhs->_ty->_is_lvalue) { error(cs, "Value can only be assigned to lvalue"); }

  from = TypeSystem::ConvertTo(cs, from, rhs->_ty, lhs->_ty);
  builder->CreateStore(from, to);
  p->_llvm_value = to;
  return to;
}

static Value *codegen_cast(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  auto lhs = p->_children[0];
  auto *dest_type = to_llvm_type(cs, p->_children[1]->_ty);
  Value *val = codegen(cs, lhs);
  Value *ret = nullptr;
  val = TypeSystem::ConvertTo(cs, val, lhs->_ty, p->_children[1]->_ty);
  if (lhs->_ty->_is_lvalue) {
    ret = create_block_alloca(builder->GetInsertBlock(), dest_type, 1, "casted");
    builder->CreateStore(val, ret);
  } else { ret = val; }
  p->_llvm_value = ret;
  return ret;
}

static Value *codegen_ty(CompilerSession *cs, ASTTyPtr p) {
  auto *builder = cs->_builder;
  resolve_ty(cs, p); // TODO: move this to analysis phase
  Ty base = TY_GET_BASE(p->_tyty);
  Value *ret = nullptr;
  Type *type = to_llvm_type(cs, p);
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
      size_t n = p->_children.size();
      for (size_t i = 1; i < n; ++i) { values.push_back((llvm::Constant *) codegen_ty(cs, p->_children[i]->_ty)); }
      ret = ConstantStruct::get((StructType *) to_llvm_type(cs, p), values);
      break;
    }
    case Ty::POINTER:
      ret = ConstantPointerNull::get((PointerType *) type);
      break;
    case Ty::ARRAY: {
      auto *e_type = to_llvm_type(cs, p->_children[0]->_ty);
      size_t n = p->_children.size();
      ret = create_block_alloca(builder->GetInsertBlock(), e_type, n, "const_array");
      for (size_t i = 0; i < n; ++i) {
        auto *idx = builder->getInt32((unsigned) i);
        auto *e_val = codegen_ty(cs, p->_children[i]->_ty);
        auto *e_ptr = builder->CreateGEP(ret, idx);
        builder->CreateStore(e_val, e_ptr);
      }
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  p->_llvm_value = ret;
  return ret;
}

static Value *codegen_var_arg_decl(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);

  if (!p->_ty->_resolved) { error(cs, "Unknown type"); }
  codegen_ty(cs, ast_cast<ASTTy>(p->_ty));
  Type *type = to_llvm_type(cs, p->_ty);
  p->_llvm_value = create_block_alloca(builder->GetInsertBlock(), type, 1, p->_name);
  if (p->_type == ASTType::VAR_DECL) { /// don't do this for arg_decl
    auto *default_value = p->_ty->_llvm_value;
    if (default_value) { builder->CreateStore(default_value, p->_ty->_llvm_value); }
  }
  /// debug info
  {
    auto *di_builder = cs->_di_builder;
    auto *curr_di_scope = cs->get_current_di_scope();
    auto *arg_meta = to_llvm_meta(cs, p->_ty);
    auto *di_arg = di_builder->createAutoVariable(curr_di_scope,
        p->_name,
        cs->get_di_file(),
        (unsigned) p->_token->l + 1,
        (DIType *) arg_meta);
    di_builder->insertDeclare(p->_llvm_value,
        di_arg,
        cs->_di_builder->createExpression(),
        llvm::DebugLoc::get((unsigned) p->_token->l + 1, (unsigned) p->_token->c + 1, curr_di_scope),
        builder->GetInsertBlock());
  }
  return p->_llvm_value;
}

static Value *codegen_address_of(CompilerSession *cs, ASTNodePtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  auto *val = codegen(cs, p->_children[0]);
  if (is_lvalue(p->_children[0])) { /// lvalue, the val itself is a pointer to real value
    p->_llvm_value = val;
  } else { /// rvalue, create an anonymous variable, and get address of it
    p->_llvm_value = create_block_alloca(builder->GetInsertBlock(), val->getType(), 1, "anonymous");
    builder->CreateStore(val, p->_llvm_value);
  }
  return p->_llvm_value;
}

static Value *codegen_parenthesis(CompilerSession *cs, ASTNodePtr p) {
  cs->set_current_debug_location(p->_token->l, p->_token->c);
  // FIXME: multiple expressions in the parenthesis?
  p->_llvm_value = codegen(cs, p->_children[0]);
  return p->_llvm_value;
}

static Value *codegen_if(CompilerSession *cs, ASTNodePtr p) {
  // TODO: update cs->_current_token
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);

  Value *condition = codegen(cs, p->_children[0]);
  if (!condition) {
    error(cs, "Invalid condition expression ");
  }

  /// convert to bool if not
  condition = TypeSystem::ConvertTo(cs, condition, p->_children[0]->_ty, create_ty(cs, Ty::BOOL));

  /// create_ty blocks for the then (and else) clause
  Function *func = builder->GetInsertBlock()->getParent();
  BasicBlock *then_bb = BasicBlock::Create(*cs->get_context(), "then", func);
  BasicBlock *else_bb = BasicBlock::Create(*cs->get_context(), "else");
  BasicBlock *merge_bb = BasicBlock::Create(*cs->get_context(), "fi");

  auto pif = ast_cast<ASTIf>(p);
  TAN_ASSERT(pif);
  if (pif->_has_else) { builder->CreateCondBr(condition, then_bb, else_bb); }
  else { builder->CreateCondBr(condition, then_bb, merge_bb); }

  /// emit then value
  builder->SetInsertPoint(then_bb);
  codegen(cs, p->_children[1]);
  /// create a br instruction if there is no terminator instruction at the end of then
  if (!builder->GetInsertBlock()->back().isTerminator()) { builder->CreateBr(merge_bb); }
  builder->SetInsertPoint(then_bb);
  if (!then_bb->back().isTerminator()) { builder->CreateBr(merge_bb); }

  /// emit else block
  if (pif->_has_else) {
    func->getBasicBlockList().push_back(else_bb);
    builder->SetInsertPoint(else_bb);
    codegen(cs, p->_children[2]);
    /// create a br instruction if there is no terminator instruction at the end of else
    if (!builder->GetInsertBlock()->back().isTerminator()) { builder->CreateBr(merge_bb); }
    builder->SetInsertPoint(else_bb);
    if (!else_bb->back().isTerminator()) { builder->CreateBr(merge_bb); }
  }

  /// emit merge block
  func->getBasicBlockList().push_back(merge_bb);
  builder->SetInsertPoint(merge_bb);
  return nullptr;
}

static Value *codegen_func_call(CompilerSession *cs, ASTNodePtr p) {
  size_t n = p->_children.size(); /// = n_args + 1

  auto callee = ast_cast<ASTFunction>(p->_children[0]);
  TAN_ASSERT(callee);

  /// args
  vector<Value *> arg_vals;
  for (size_t i = 1; i < n; ++i) {
    auto *a = codegen(cs, p->_children[i]);
    if (!a) { error(cs, "Invalid function call argument"); }

    /// implicit cast
    auto expected_ty = callee->get_arg(i)->_ty;
    a = TypeSystem::ConvertTo(cs, a, p->_children[i]->_ty, expected_ty);
    arg_vals.push_back(a);
  }
  p->_llvm_value = cs->_builder->CreateCall(callee->_func, arg_vals);
  return p->_llvm_value;
}

static Value *codegen_func_prototype(CompilerSession *cs, ASTFunctionPtr p, bool import = false) {
  Type *ret_type = to_llvm_type(cs, p->_children[0]->_ty);
  vector<Type *> arg_types{};
  /// set function arg types
  for (size_t i = 1; i < p->_children.size() - !p->_is_external; ++i) {
    arg_types.push_back(to_llvm_type(cs, p->_children[i]->_ty));
  }
  /// create function prototype
  FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
  auto linkage = Function::InternalLinkage;
  if (p->_is_external) { linkage = Function::ExternalWeakLinkage; }
  if (p->_is_public) {
    if (import) {
      linkage = Function::ExternalWeakLinkage;
    } else {
      linkage = Function::ExternalLinkage;
    }
  }
  Function *func = Function::Create(FT, linkage, p->_name, cs->get_module());
  func->setCallingConv(llvm::CallingConv::C);

  /// set argument names
  auto args = func->args().begin();
  for (size_t i = 1, j = 0; i < p->_children.size() - !p->_is_external; ++i, ++j) {
    /// rename this since the real function argument is stored as variables
    (args + j)->setName("_" + p->_children[i]->_name);
  }
  p->_llvm_value = func;
  return func;
}

static Value *codegen_func_decl(CompilerSession *cs, ASTFunctionPtr p) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(p->_token->l, p->_token->c);

  auto ret_ty = p->_children[0]->_ty;
  Metadata *ret_meta = to_llvm_meta(cs, ret_ty);

  /// generate prototype
  auto *F = (Function *) codegen_func_prototype(cs, p);

  cs->push_scope(p->_scope); /// push scope
  /// set function arg types
  vector<Metadata *> arg_metas;
  for (size_t i = 1; i < p->_children.size() - !p->_is_external; ++i) {
    auto ty = p->_children[i]->_ty;
    arg_metas.push_back(to_llvm_meta(cs, ty));
  }

  /// get function name
  str func_name = p->_name;
  /// function implementation
  if (!p->_is_external) {
    /// create a new basic block to start insertion into
    BasicBlock *main_block = BasicBlock::Create(*cs->get_context(), "func_entry", F);
    builder->SetInsertPoint(main_block);

    /// debug information
    DIScope *di_scope = cs->get_current_di_scope();
    auto *di_file = cs->get_di_file();
    auto *di_func_t = create_function_type(cs, ret_meta, arg_metas);
    DISubprogram *subprogram = cs->_di_builder
        ->createFunction(di_scope,
            func_name,
            func_name,
            di_file,
            (unsigned) p->_token->l + 1,
            di_func_t,
            (unsigned) p->_token->l + 1,
            DINode::FlagPrototyped,
            DISubprogram::SPFlagDefinition,
            nullptr,
            nullptr,
            nullptr);
    F->setSubprogram(subprogram);
    cs->push_di_scope(subprogram);
    /// reset debug emit location
    builder->SetCurrentDebugLocation(DebugLoc());

    /// add all function arguments to scope
    size_t i = 1;
    for (auto &a : F->args()) {
      auto arg_name = p->_children[i]->_name;
      auto *arg_val = codegen(cs, p->_children[i]);
      builder->CreateStore(&a, arg_val);
      /// create a debug descriptor for the arguments
      auto *arg_meta = to_llvm_meta(cs, p->_children[i]->_ty);
      llvm::DILocalVariable *di_arg = cs->_di_builder
          ->createParameterVariable(subprogram,
              arg_name,
              (unsigned) i + 1,
              di_file,
              (unsigned) p->_token->l + 1,
              (DIType *) arg_meta,
              true);
      cs->_di_builder
          ->insertDeclare(arg_val,
              di_arg,
              cs->_di_builder->createExpression(),
              llvm::DebugLoc::get((unsigned) p->_token->l + 1, (unsigned) p->_token->c + 1, subprogram),
              builder->GetInsertBlock());
      ++i;
    }

    /// set debug emit location to function body
    builder->SetCurrentDebugLocation(DebugLoc::get((unsigned) p->_children.back()->_token->l + 1,
        (unsigned) p->_children.back()->_token->c + 1,
        subprogram));
    /// generate function body
    auto body = p->_children.back();
    codegen(cs, body);

    /// create a return instruction if there is none, the return value is the default value of the return type
    if (!builder->GetInsertBlock()->back().isTerminator()) {
      if (ret_ty->_tyty == Ty::VOID) {
        builder->CreateRetVoid();
      } else {
        auto *ret_val = ret_ty->_llvm_value;
        TAN_ASSERT(ret_val);
        builder->CreateRet(ret_val);
      }
    }
    cs->pop_di_scope();
    /// restore parent code block
    builder->SetInsertPoint(main_block);
  }
  cs->pop_scope(); /// pop scope
  return nullptr;
}

Value *codegen(CompilerSession *cs, ASTNodePtr p) {
  Value *ret = nullptr;
  switch (p->_type) {
    case ASTType::PROGRAM:
      for (const auto &e : p->_children) { codegen(cs, e); }
      ret = nullptr;
      break;
      ///////////////////////// binary ops ///////////////////////////
    case ASTType::SUM:
    case ASTType::SUBTRACT:
    case ASTType::MULTIPLY:
    case ASTType::DIVIDE:
    case ASTType::MOD:
      ret = codegen_arithmetic(cs, p);
      break;
    case ASTType::GT:
    case ASTType::GE:
    case ASTType::LT:
    case ASTType::LE:
    case ASTType::EQ:
    case ASTType::NE:
      ret = codegen_comparison(cs, p);
      break;
    case ASTType::ASSIGN:
      ret = codegen_assignment(cs, p);
      break;
    case ASTType::CAST:
      ret = codegen_cast(cs, p);
      break;
      ////////////////////////////// literals ////////////////////////
    case ASTType::ARRAY_LITERAL:
    case ASTType::NUM_LITERAL:
    case ASTType::CHAR_LITERAL:
    case ASTType::STRING_LITERAL:
      ret = p->_llvm_value = codegen(cs, p->_ty);
      break;
      ////////////////////////////// prefix //////////////////////////
    case ASTType::ADDRESS_OF:
      ret = codegen_address_of(cs, p);
      break;
    case ASTType::RET:
      ret = codegen_return(cs, p);
      break;
    case ASTType::LNOT:
      ret = codegen_lnot(cs, p);
      break;
    case ASTType::BNOT:
      ret = codegen_bnot(cs, p);
      break;
      ///////////////////////////// other ////////////////////////////
    case ASTType::FUNC_DECL: {
      auto pf = ast_cast<ASTFunction>(p);
      TAN_ASSERT(pf);
      ret = codegen_func_decl(cs, pf);
      break;
    }
    case ASTType::FUNC_CALL:
      ret = codegen_func_call(cs, p);
      break;
    case ASTType::IF:
      ret = codegen_if(cs, p);
      break;
    case ASTType::VAR_DECL:
    case ASTType::ARG_DECL:
      ret = codegen_var_arg_decl(cs, p);
      break;
    case ASTType::TY:
      ret = codegen_ty(cs, ast_cast<ASTTy>(p));
      break;
    case ASTType::PARENTHESIS:
      ret = codegen_parenthesis(cs, p);
      break;
    case ASTType::ID:
      ret = codegen(cs, p->_children[0]);
      break;
    default:
      break;
  }
  return ret;
}

} // namespace tanlang
