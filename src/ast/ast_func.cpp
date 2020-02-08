#include "src/ast/ast_func.h"
#include "parser.h"
#include "src/llvm_include.h"
#include "token.h"

namespace tanlang {

// FIXME: move this to common.h
static Type *type_from_string(const std::string &type_name, CompilerSession *parser_context) {
  Type *t = nullptr;
  if (type_name == "int") {
    t = parser_context->get_builder()->getInt32Ty();
  } else if (type_name == "float") {
    t = parser_context->get_builder()->getFloatTy();
  }
  return t;
}

Value *ASTFunction::codegen(CompilerSession *compiler_session) {
  // new scope
  compiler_session->push_scope();
  // make function prototype
  Type *float_type = compiler_session->get_builder()->getFloatTy();
  // std::vector<Type *> arg_types(2, float_type);
  std::vector<Type *> arg_types;
  // set function arg types
  for (size_t i = 2; i < _children.size() - !_is_external; ++i) {
    std::shared_ptr<ASTTypeName> type_name = std::reinterpret_pointer_cast<ASTTypeName>(_children[i]->_children[1]);
    arg_types.push_back(type_from_string(type_name->_name, compiler_session));
  }

  // get function name
  std::shared_ptr<ASTIdentifier> fname = std::reinterpret_pointer_cast<ASTIdentifier>(_children[1]);
  std::string func_name = fname->_name;

  // create function
  FunctionType *FT = FunctionType::get(float_type, arg_types, false);
  // FIXME: external linkage
  Function *F = Function::Create(FT, Function::ExternalLinkage, func_name, compiler_session->get_module().get());
  // F->setCallingConv(llvm::CallingConv::C);

  // set argument names
  auto args = F->args().begin();
  for (size_t i = 2, j = 0; i < _children.size() - !_is_external; ++i, ++j) {
    std::shared_ptr<ASTIdentifier> arg_name = std::reinterpret_pointer_cast<ASTIdentifier>(_children[i]->_children[0]);
    (args + j)->setName(arg_name->_name);
  }

  // function implementation
  // create a new basic block to start insertion into
  if (!_is_external) {
    BasicBlock *main_block = BasicBlock::Create(*compiler_session->get_context(), "func_entry", F);
    compiler_session->get_builder()->SetInsertPoint(main_block);
    compiler_session->set_code_block(main_block); // set current scope's code block
  }

  // add all function arguments to scope
  for (auto &Arg : F->args()) {
    compiler_session->add(Arg.getName(), &Arg);
  }

  if (!_is_external) {
    _children[_children.size() - 1]->codegen(compiler_session);
  }

  // validate the generated code, checking for consistency
  verifyFunction(*F);
  compiler_session->pop_scope(); // pop scope
  compiler_session->get_builder()->SetInsertPoint(compiler_session->get_code_block()); // restore parent code block
  return nullptr;
}

Value *ASTFunctionCall::codegen(CompilerSession *compiler_session) {
  // Look up the name in the global module table.
  Function *func = compiler_session->get_module()->getFunction(_name);
  if (!func) {
    throw std::runtime_error("Unknown function call: " + _name);
  }

  size_t n_args = _children.size();
  // If argument mismatch error.
  if (func->arg_size() != n_args) {
    throw std::runtime_error("Invalid number of arguments: " + std::to_string(n_args));
  }

  // push args
  std::vector<Value *> args_value;
  for (size_t i = 0; i < n_args; ++i) {
    args_value.push_back(_children[i]->codegen(compiler_session));
    if (!args_value.back()) { return nullptr; }
  }
  return compiler_session->get_builder()->CreateCall(func, args_value, "call_tmp");
}

} // namespace tanlang
