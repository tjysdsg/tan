#include "src/ast/ast_func.h"
#include "parser.h"
#include "src/llvm_include.h"
#include "token.h"

namespace tanlang {

ASTFunction::ASTFunction(Token *token) : ASTNode(ASTType::FUNC, 0, 0, token) {}

static Type *type_from_string(const std::string &type_name, CompilerSession *parser_context) {
  Type *t = nullptr;
  if (type_name == "int") {
    t = parser_context->get_builder()->getInt32Ty();
  } else if (type_name == "float") {
    t = parser_context->get_builder()->getFloatTy();
  }
  return t;
}

Value *ASTFunction::codegen(CompilerSession *parser_context) {
  // make function prototype
  Type *float_type = parser_context->get_builder()->getFloatTy();
  // std::vector<Type *> arg_types(2, float_type);
  std::vector<Type *> arg_types;
  // set function arg types
  for (size_t i = 2; i < _children.size() - 1; ++i) {
    std::shared_ptr<ASTTypeName> type_name = std::reinterpret_pointer_cast<ASTTypeName>(_children[i]->_children[1]);
    arg_types.push_back(type_from_string(type_name->_name, parser_context));
  }

  // get function name
  std::shared_ptr<ASTIdentifier> fname = std::reinterpret_pointer_cast<ASTIdentifier>(_children[1]);
  std::string func_name = fname->_name;

  // create function
  FunctionType *FT = FunctionType::get(float_type, arg_types, false);
  Function *F = Function::Create(FT, Function::ExternalLinkage, func_name, parser_context->get_module().get());

  // set argument names
  auto args = F->args().begin();
  for (size_t i = 2, j = 0; i < _children.size() - 1; ++i, ++j) {
    std::shared_ptr<ASTIdentifier> arg_name = std::reinterpret_pointer_cast<ASTIdentifier>(_children[i]->_children[0]);
    (args + j)->setName(arg_name->_name);
  }

  // function implementation
  // create a new basic block to start insertion into
  BasicBlock *main_block = BasicBlock::Create(*parser_context->get_context(), "entry", F);
  parser_context->get_builder()->SetInsertPoint(main_block);

  parser_context->push_scope(); // new scope
  // add all function arguments to scope
  for (auto &Arg : F->args()) {
    parser_context->add(Arg.getName(), &Arg);
  }

  _children[_children.size() - 1]->codegen(parser_context);

  // validate the generated code, checking for consistency
  verifyFunction(*F);
  parser_context->pop_scope(); // pop scope
  return nullptr;
}

}
