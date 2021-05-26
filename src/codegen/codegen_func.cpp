#include "src/codegen/code_generator_impl.h"
#include "src/llvm_include.h"
#include "compiler_session.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_ty.h"
#include "src/analysis/type_system.h"

using namespace tanlang;

Value *CodeGeneratorImpl::codegen_func_call(const ASTNodePtr &p) {
  ptr<ASTFunctionCall> f = ast_must_cast<ASTFunctionCall>(p);

  ASTFunctionPtr callee = f->_callee;
  size_t n = callee->get_n_args();

  /// args
  vector<Value *> arg_vals;
  for (size_t i = 0; i < n; ++i) {
    auto *a = codegen(p->get_child_at<ASTNode>(i));
    if (!a) {
      report_error(p->get_child_at<ASTNode>(i), "Invalid function call argument");
    }

    /// implicit cast
    auto expected_ty = callee->get_arg(i)->_ty;
    a = TypeSystem::ConvertTo(_cs, a, p->get_child_at<ASTNode>(i)->_ty, expected_ty);
    arg_vals.push_back(a);
  }
  p->_llvm_value = _cs->_builder->CreateCall(callee->_func, arg_vals);
  return p->_llvm_value;
}

Value *CodeGeneratorImpl::codegen_func_prototype(const ASTFunctionPtr &p, bool import) {
  Type *ret_type = TypeSystem::ToLLVMType(_cs, p->get_child_at<ASTNode>(0)->_ty);
  vector<Type *> arg_types{};
  /// set function arg types
  for (size_t i = 1; i < p->get_children_size() - !p->_is_external; ++i) {
    arg_types.push_back(TypeSystem::ToLLVMType(_cs, p->get_child_at<ASTNode>(i)->_ty));
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
  Function *func = Function::Create(FT, linkage, p->get_data<str>(), _cs->get_module());
  func->setCallingConv(llvm::CallingConv::C);

  /// set argument names
  auto args = func->args().begin();
  for (size_t i = 1, j = 0; i < p->get_children_size() - !p->_is_external; ++i, ++j) {
    /// rename this since the real function argument is stored as variables
    (args + j)->setName("_" + p->get_child_at<ASTNode>(i)->get_data<str>());
  }
  p->_llvm_value = func;
  p->_func = func;
  return func;
}

Value *CodeGeneratorImpl::codegen_func_decl(const ASTFunctionPtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto ret_ty = p->get_child_at<ASTNode>(0)->_ty;
  Metadata *ret_meta = TypeSystem::ToLLVMMeta(_cs, ret_ty);

  /// generate prototype
  auto *F = (Function *) codegen_func_prototype(p);

  _cs->push_scope(p->get_scope()); /// push scope
  /// set function arg types
  vector<Metadata *> arg_metas;
  for (size_t i = 1; i < p->get_children_size() - !p->_is_external; ++i) {
    auto ty = p->get_child_at<ASTNode>(i)->_ty;
    arg_metas.push_back(TypeSystem::ToLLVMMeta(_cs, ty));
  }

  /// get function name
  str func_name = p->get_data<str>();
  /// function implementation
  if (!p->_is_external) {
    /// create a new basic block to start insertion into
    BasicBlock *main_block = BasicBlock::Create(*_cs->get_context(), "func_entry", F);
    builder->SetInsertPoint(main_block);

    /// debug information
    DIScope *di_scope = _cs->get_current_di_scope();
    auto *di_file = _cs->get_di_file();
    auto *di_func_t = TypeSystem::CreateFunctionDIType(_cs, ret_meta, arg_metas);
    DISubprogram *subprogram = _cs->_di_builder
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
    _cs->push_di_scope(subprogram);
    /// reset debug emit location
    builder->SetCurrentDebugLocation(DebugLoc());

    /// add all function arguments to scope
    size_t i = 1;
    for (auto &a : F->args()) {
      auto arg_name = p->get_child_at<ASTNode>(i)->get_data<str>();
      auto *arg_val = codegen(p->get_child_at<ASTNode>(i));
      builder->CreateStore(&a, arg_val);
      /// create a debug descriptor for the arguments
      auto *arg_meta = TypeSystem::ToLLVMMeta(_cs, p->get_child_at<ASTNode>(i)->_ty);
      llvm::DILocalVariable *di_arg = _cs->_di_builder
          ->createParameterVariable(subprogram,
              arg_name,
              (unsigned) i + 1,
              di_file,
              (unsigned) p->get_line(),
              (DIType *) arg_meta,
              true);
      _cs->_di_builder
          ->insertDeclare(arg_val,
              di_arg,
              _cs->_di_builder->createExpression(),
              llvm::DebugLoc::get((unsigned) p->get_line(), (unsigned) p->get_col(), subprogram),
              builder->GetInsertBlock());
      ++i;
    }

    /// set debug emit location to function body
    builder->SetCurrentDebugLocation(llvm::DebugLoc::get((unsigned) p->get_children().back()->get_line(),
        (unsigned) p->get_children().back()->get_col(),
        subprogram));
    /// generate function body
    auto body = ast_must_cast<ASTNode>(p->get_children().back());
    codegen(body);

    /// create a return instruction if there is none, the return value is the default value of the return type
    if (!builder->GetInsertBlock()->back().isTerminator()) {
      if (ret_ty->_tyty == Ty::VOID) {
        builder->CreateRetVoid();
      } else {
        auto *ret_val = codegen_ty(ret_ty);
        TAN_ASSERT(ret_val);
        builder->CreateRet(ret_val);
      }
    }
    _cs->pop_di_scope();
    /// restore parent code block
    builder->SetInsertPoint(main_block);
  }
  _cs->pop_scope(); /// pop scope
  return nullptr;
}
