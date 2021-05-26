#include "src/codegen/code_generator_impl.h"
#include "src/llvm_include.h"
#include "compiler_session.h"

using namespace tanlang;

Value *CodeGeneratorImpl::codegen_func_call(ASTNodePtr p) {
  size_t n = p->_children.size(); /// = n_args + 1

  auto callee = ast_cast<ASTFunction>(p->_children[0]);
  TAN_ASSERT(callee);

  /// args
  vector<Value *> arg_vals;
  for (size_t i = 1; i < n; ++i) {
    auto *a = codegen(cs, p->_children[i]);
    if (!a) { report_error(cs, p->_children[i], "Invalid function call argument"); }

    /// implicit cast
    auto expected_ty = callee->get_arg(i)->_ty;
    a = TypeSystem::ConvertTo(cs, a, p->_children[i]->_ty, expected_ty);
    arg_vals.push_back(a);
  }
  p->_llvm_value = cs->_builder->CreateCall(callee->_func, arg_vals);
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_func_prototype(ASTFunctionPtr p, bool import) {
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
  p->_func = func;
  return func;
}

Value *CodeGeneratorImpl::codegen_func_decl(ASTFunctionPtr p) {
  auto *builder = cs->_builder;
  set_current_debug_location(cs, p);

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
            (unsigned) p->get_line(),
            di_func_t,
            (unsigned) p->get_col(),
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
              (unsigned) p->get_line(),
              (DIType *) arg_meta,
              true);
      cs->_di_builder
          ->insertDeclare(arg_val,
              di_arg,
              cs->_di_builder->createExpression(),
              llvm::DebugLoc::get((unsigned) p->get_line(), (unsigned) p->get_col(), subprogram),
              builder->GetInsertBlock());
      ++i;
    }

    /// set debug emit location to function body
    builder->SetCurrentDebugLocation(llvm::DebugLoc::get((unsigned) p->_children.back()->get_line(),
        (unsigned) p->_children.back()->get_col(),
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

