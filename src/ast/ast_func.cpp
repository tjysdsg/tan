#include "src/ast/ast_func.h"
#include "parser.h"
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Function.h>
#include "token.h"

namespace tanlang {
using llvm::Type;
using llvm::verifyFunction;
using llvm::FunctionType;
using llvm::Function;

ASTFunction::ASTFunction(Token *token) : ASTNode(ASTType::FUNC, 0, 0, token) {}

void ASTFunction::nud(Parser *parser) {
  _children.push_back(std::make_shared<ASTNode>()); // function return type, set later
  _children.push_back(parser->next_expression(PREC_HIGHEST)); // function name
  parser->advance(TokenType::PUNCTUATION, "(");
  // if the argument list isn't empty
  if (parser->get_curr_token()->type != TokenType::PUNCTUATION || parser->get_curr_token()->value != ")") {
    size_t token_size = parser->_tokens.size();
    while (parser->_curr_token < token_size) {
      std::shared_ptr<ASTNode> arg = std::make_shared<ASTArgDef>(parser->get_curr_token());
      arg->nud(parser);
      _children.push_back(arg);
      if (parser->get_curr_token()->type == TokenType::PUNCTUATION && parser->get_curr_token()->value == ",") {
        ++parser->_curr_token;
      } else { break; }
    }
  }
  parser->advance(TokenType::PUNCTUATION, ")");
  parser->advance(TokenType::PUNCTUATION, ":");
  // figure out function return type
  _children[0] = parser->next_node();

  // function code
  std::shared_ptr<ASTNode> code = std::make_shared<ASTStatement>(parser->get_curr_token());
  code->nud(parser);
  _children.push_back(code);
}

static Type *type_from_string(const std::string &type_name, ParserContext *parser_context) {
  Type *t = nullptr;
  if (type_name == "int") {
    t = parser_context->_builder->getInt32Ty();
  } else if (type_name == "float") {
    t = parser_context->_builder->getFloatTy();
  }
  return t;
}

Value *ASTFunction::codegen(ParserContext *parser_context) {
  // make function prototype
  Type *float_type = parser_context->_builder->getFloatTy();
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
  Function *F = Function::Create(FT, Function::ExternalLinkage, func_name, *parser_context->_module);

  // set argument names
  auto args = F->args().begin();
  for (size_t i = 2, j = 0; i < _children.size() - 1; ++i, ++j) {
    std::shared_ptr<ASTIdentifier> arg_name = std::reinterpret_pointer_cast<ASTIdentifier>(_children[i]->_children[0]);
    (args + j)->setName(arg_name->_name);
  }

  // function implementation
  // create a new basic block to start insertion into
  BasicBlock *main_block = BasicBlock::Create(*parser_context->_context, "entry", F);
  parser_context->_builder->SetInsertPoint(main_block);

  // TODO: create a new scope
  // add all function arguments to scope
  for (auto &Arg : F->args()) {
    parser_context->add_variable(Arg.getName(), &Arg);
  }

  for (const auto &child : _children) {
    child->codegen(parser_context);
  }

  // validate the generated code, checking for consistency
  verifyFunction(*F);
  return nullptr;
}

}
