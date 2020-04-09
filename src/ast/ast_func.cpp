#include "src/ast/ast_func.h"
#include "parser.h"
#include "src/ast/common.h"

namespace tanlang {

Value *ASTFunction::codegen(CompilerSession *compiler_session) {
  /// new scope
  compiler_session->push_scope();
  /// make function prototype
  Type *ret_type = ast_cast<ASTTy>(_children[0])->to_llvm_type(compiler_session);
  std::vector<Type *> arg_types;
  /// set function arg types
  for (size_t i = 2; i < _children.size() - !_is_external; ++i) {
    std::shared_ptr<ASTTy> type_name = ast_cast<ASTTy>(_children[i]->_children[1]);
    arg_types.push_back(type_name->to_llvm_type(compiler_session));
  }

  /// get function name
  std::shared_ptr<ASTIdentifier> fname = ast_cast<ASTIdentifier>(_children[1]);
  std::string func_name = fname->get_name();

  /// create function
  FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
  // FIXME: external linkage
  Function *F = Function::Create(FT, Function::ExternalLinkage, func_name, compiler_session->get_module().get());
  F->setCallingConv(llvm::CallingConv::C);

  /// set argument names
  auto args = F->args().begin();
  for (size_t i = 2, j = 0; i < _children.size() - !_is_external; ++i, ++j) {
    /// rename this since the real function argument is stored as variables
    (args + j)->setName("_" + _children[i]->get_name());
  }

  /// function implementation
  /// create a new basic block to start insertion into
  if (!_is_external) {
    BasicBlock *main_block = BasicBlock::Create(*compiler_session->get_context(), "func_entry", F);
    compiler_session->get_builder()->SetInsertPoint(main_block);
    compiler_session->set_code_block(main_block); /// set current scope's code block

    /// add all function arguments to scope
    size_t i = 2;
    for (auto &a : F->args()) {
      auto arg_name = _children[i]->get_name();
      Value *arg_val = create_block_alloca(compiler_session->get_builder()->GetInsertBlock(), a.getType(), arg_name);
      compiler_session->get_builder()->CreateStore(&a, arg_val);
      ast_cast<ASTVarDecl>(_children[i])->_llvm_value = arg_val;
      compiler_session->add(arg_name, _children[i]);
      ++i;
    }

    /// generate function body
    _children[_children.size() - 1]->codegen(compiler_session);

    /// create a return instruction if there is none
    if (!(main_block->back().getOpcode() & llvm::Instruction::Ret)) {
      compiler_session->get_builder()->CreateRetVoid();
    }
  }

  /// validate the generated code, checking for consistency
  verifyFunction(*F);
  compiler_session->get_builder()->SetInsertPoint(compiler_session->get_code_block()); /// restore parent code block
  compiler_session->pop_scope(); /// pop scope
  return nullptr;
}

Value *ASTFunctionCall::codegen(CompilerSession *compiler_session) {
  /// look up the function name in the global module table
  Function *func = compiler_session->get_module()->getFunction(_name);
  if (!func) {
    throw std::runtime_error("Unknown function call: " + _name);
  }

  size_t n_args = _children.size();
  /// argument mismatch
  if (func->arg_size() != n_args) {
    throw std::runtime_error("Invalid number of arguments: " + std::to_string(n_args));
  }

  /// push args
  std::vector<Value *> args_value;
  for (size_t i = 0; i < n_args; ++i) {
    auto *a = _children[i]->codegen(compiler_session);
    if (_children[i]->is_lvalue()) {
      a = compiler_session->get_builder()->CreateLoad(a);
    }
    args_value.push_back(a);
    if (!args_value.back()) { return nullptr; }
  }
  return compiler_session->get_builder()->CreateCall(func, args_value);
}

} // namespace tanlang
