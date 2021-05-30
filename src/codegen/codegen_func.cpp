#include "src/codegen/code_generator_impl.h"
#include "src/llvm_include.h"
#include "compiler_session.h"
#include "src/ast/ast_type.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"
#include "src/analysis/type_system.h"

using namespace tanlang;

Value *CodeGeneratorImpl::codegen_func_call(const ASTBasePtr &_p) {
  auto p = ast_must_cast<FunctionCall>(_p);

  FunctionDeclPtr callee = p->_callee;
  size_t n = callee->get_n_args();

  /// args
  vector<Value *> arg_vals;
  for (size_t i = 0; i < n; ++i) {
    auto actual_arg = p->_args[i];
    auto *a = codegen(actual_arg);
    if (!a) {
      report_error(actual_arg, "Invalid function call argument");
    }

    /// implicit cast
    auto expected_ty = callee->get_arg_type(i);
    a = TypeSystem::ConvertTo(_cs, a, actual_arg->get_type(), expected_ty);
    arg_vals.push_back(a);
  }
  return p->_llvm_value = _cs->_builder->CreateCall(codegen(callee), arg_vals);
}

Value *CodeGeneratorImpl::codegen_func_prototype(const FunctionDeclPtr &p, bool import) {
  Type *ret_type = TypeSystem::ToLLVMType(_cs, p->get_ret_ty());

  /// set function arg types
  vector<Type *> arg_types{};
  for (size_t i = 0; i < p->get_n_args(); ++i) {
    arg_types.push_back(TypeSystem::ToLLVMType(_cs, p->get_arg_type(i)));
  }

  /// create function prototype
  FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
  auto linkage = Function::InternalLinkage;
  if (p->is_external()) {
    linkage = Function::ExternalWeakLinkage;
  }
  if (p->is_public()) {
    if (import) {
      linkage = Function::ExternalWeakLinkage;
    } else {
      linkage = Function::ExternalLinkage;
    }
  }
  Function *func = Function::Create(FT, linkage, p->get_name(), _cs->get_module());
  func->setCallingConv(llvm::CallingConv::C);

  /// set argument names
  auto args = func->args().begin();
  for (size_t i = 0; i < p->get_n_args(); ++i) {
    /// rename this since the real function argument is stored as variables
    (args + i)->setName("_" + p->get_arg_name(i));
  }
  return p->_llvm_value = func;
}

Value *CodeGeneratorImpl::codegen_func_decl(const FunctionDeclPtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  auto ret_ty = p->get_ret_ty();
  Metadata *ret_meta = TypeSystem::ToLLVMMeta(_cs, ret_ty);

  /// generate prototype
  auto *F = (Function *) codegen_func_prototype(p);

  /// push scope
  _cs->push_scope(p->get_scope());

  /// set function arg types
  vector<Metadata *> arg_metas;
  for (size_t i = 0; i < p->get_n_args(); ++i) {
    auto ty = p->get_arg_type(i);
    arg_metas.push_back(TypeSystem::ToLLVMMeta(_cs, ty));
  }

  /// get function name
  str func_name = p->get_name();

  /// function implementation
  if (!p->is_external()) {
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
    size_t i = 0;
    for (auto &a : F->args()) {
      auto arg_name = p->get_arg_name(i);
      auto *arg_val = codegen(p->get_arg_decls()[i]);
      builder->CreateStore(&a, arg_val);
      /// create a debug descriptor for the arguments
      auto *arg_meta = TypeSystem::ToLLVMMeta(_cs, p->get_arg_type(i));
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
    builder->SetCurrentDebugLocation(llvm::DebugLoc::get((unsigned) p->get_body()->get_line(),
        (unsigned) p->get_body()->get_col(),
        subprogram));

    /// generate function body
    codegen(p->get_body());

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
