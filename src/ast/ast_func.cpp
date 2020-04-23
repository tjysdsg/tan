#include "src/ast/ast_func.h"
#include "parser.h"
#include "src/ast/common.h"
#include "src/ast/type_system.h"
#include "stack_trace.h"
#include "intrinsic.h"

namespace tanlang {

Value *ASTFunction::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  /// new scope
  auto scope = compiler_session->push_scope();

  Metadata *ret_meta = _children[0]->to_llvm_meta(compiler_session);
  std::vector<Metadata *> arg_metas;
  /// set function arg types
  for (size_t i = 2; i < _children.size() - !_is_external; ++i) {
    std::shared_ptr<ASTTy> type_name = ast_cast<ASTTy>(_children[i]->_children[1]);
    arg_metas.push_back(type_name->to_llvm_meta(compiler_session));
  }
  /// generate prototype
  Function *F = (Function *) codegen_prototype(compiler_session);
  /// get function name
  std::string func_name = _children[1]->get_name();
  /// function implementation
  if (!_is_external) {
    /// create a new basic block to start insertion into
    BasicBlock *main_block = BasicBlock::Create(*compiler_session->get_context(), "func_entry", F);
    compiler_session->get_builder()->SetInsertPoint(main_block);
    compiler_session->set_code_block(main_block);

    /// debug information
    DIScope *di_scope = compiler_session->get_current_di_scope();
    auto *di_file = compiler_session->get_di_file();
    auto *di_func_t = create_function_type(compiler_session, ret_meta, arg_metas);
    DISubprogram *subprogram = compiler_session->get_di_builder()
        ->createFunction(di_scope,
            func_name,
            func_name,
            di_file,
            (unsigned) _token->l + 1,
            di_func_t,
            (unsigned) _token->l + 1,
            DINode::FlagPrototyped,
            DISubprogram::SPFlagDefinition,
            nullptr,
            nullptr,
            nullptr);
    F->setSubprogram(subprogram);
    compiler_session->push_di_scope(subprogram);
    /// reset debug emit location
    compiler_session->get_builder()->SetCurrentDebugLocation(DebugLoc());

    /// set initial stack trace for the main function
    if (func_name == "main") {
      Intrinsic::RuntimeInit(compiler_session);
      auto stack_trace = std::make_shared<StackTrace>();
      stack_trace->_filename = _parser->get_filename();
      stack_trace->_src = _token->line->code;
      stack_trace->_lineno = _token->l + 1;
      codegen_push_stack_trace(compiler_session, stack_trace);
    }

    /// add all function arguments to scope
    size_t i = 2;
    for (auto &a : F->args()) {
      auto arg_name = _children[i]->get_name();
      Value *arg_val = create_block_alloca(compiler_session->get_builder()->GetInsertBlock(), a.getType(), arg_name);
      compiler_session->get_builder()->CreateStore(&a, arg_val);
      ast_cast<ASTVarDecl>(_children[i])->_llvm_value = arg_val;
      compiler_session->add(arg_name, _children[i]);

      /// create a debug descriptor for the arguments
      auto *arg_meta = ast_cast<ASTTy>(_children[i]->_children[1])->to_llvm_meta(compiler_session);
      llvm::DILocalVariable *di_arg = compiler_session->get_di_builder()
          ->createParameterVariable(subprogram,
              arg_name,
              (unsigned) i + 1,
              di_file,
              (unsigned) _token->l + 1,
              arg_meta,
              true);
      compiler_session->get_di_builder()
          ->insertDeclare(arg_val,
              di_arg,
              compiler_session->get_di_builder()->createExpression(),
              llvm::DebugLoc::get((unsigned) _token->l + 1, (unsigned) _token->c + 1, subprogram),
              compiler_session->get_builder()->GetInsertBlock());
      ++i;
    }

    /// set debug emit location to function body
    compiler_session->get_builder()
        ->SetCurrentDebugLocation(DebugLoc::get((unsigned) _children[_children.size() - 1]->_token->l + 1,
            (unsigned) _children[_children.size() - 1]->_token->c + 1,
            di_scope));
    /// generate function body
    _children[_children.size() - 1]->codegen(compiler_session);

    /// create a return instruction if there is none
    if (!(compiler_session->get_builder()->GetInsertBlock()->back().getOpcode() & llvm::Instruction::Ret)) {
      compiler_session->get_builder()->CreateRetVoid();
    }
    compiler_session->pop_di_scope();
  }
  verifyFunction(*F);
  /// function pass
  compiler_session->get_function_pass_manager()->run(*F);
  /// restore parent code block
  compiler_session->get_builder()->SetInsertPoint(compiler_session->get_code_block());
  /// pop scope
  compiler_session->pop_scope();
  return nullptr;
}

Value *ASTFunction::codegen_prototype(CompilerSession *compiler_session, bool import) {
  Type *ret_type = _children[0]->to_llvm_type(compiler_session);
  std::vector<Type *> arg_types;
  /// set function arg types
  for (size_t i = 2; i < _children.size() - !_is_external; ++i) {
    std::shared_ptr<ASTTy> type_name = ast_cast<ASTTy>(_children[i]->_children[1]);
    arg_types.push_back(type_name->to_llvm_type(compiler_session));
  }

  /// get function name
  std::shared_ptr<ASTIdentifier> fname = ast_cast<ASTIdentifier>(_children[1]);
  std::string func_name = fname->get_name();

  /// create function prototype
  FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
  auto linkage = Function::InternalLinkage;
  if (_is_external) { linkage = Function::ExternalWeakLinkage; }
  if (_is_public) {
    if (import) {
      linkage = Function::ExternalWeakLinkage;
    } else {
      linkage = Function::ExternalLinkage;
    }
  }
  Function *F = Function::Create(FT, linkage, func_name, compiler_session->get_module().get());
  F->setCallingConv(llvm::CallingConv::C);

  /// set argument names
  auto args = F->args().begin();
  for (size_t i = 2, j = 0; i < _children.size() - !_is_external; ++i, ++j) {
    /// rename this since the real function argument is stored as variables
    (args + j)->setName("_" + _children[i]->get_name());
  }
  return F;
}

Value *ASTFunctionCall::codegen(CompilerSession *compiler_session) {
  /// look up the function name in the global module table
  Function *func = compiler_session->get_module()->getFunction(_name);
  if (!func) { throw std::runtime_error("Unknown function call: " + _name); }

  size_t n_args = _children.size();
  /// argument mismatch
  if (func->arg_size() != n_args) {
    throw std::runtime_error("Invalid number of arguments: " + std::to_string(n_args));
  }

  /// push args
  std::vector<Value *> args_value;
  auto func_arg = func->args().begin();
  for (size_t i = 0; i < n_args; ++i) {
    auto *a = _children[i]->codegen(compiler_session);
    a = convert_to(compiler_session, (func_arg + i)->getType(), a, _children[i]->is_lvalue());
    args_value.push_back(a);
    if (!args_value.back()) { return nullptr; }
  }
  auto stack_trace = std::make_shared<StackTrace>();
  stack_trace->_filename = _parser->get_filename();
  stack_trace->_src = _token->line->code;
  stack_trace->_lineno = _children[0]->_token->l + 1;
  codegen_push_stack_trace(compiler_session, stack_trace);
  auto *ret = compiler_session->get_builder()->CreateCall(func, args_value);
  codegen_pop_stack_trace(compiler_session);
  return ret;
}

} // namespace tanlang
