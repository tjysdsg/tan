#include <llvm/IR/Instruction.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/APFloat.h>
#include "parser.h"
#include "src/ast/ast_statement.h"
#include "src/ast/astnode.h"
#include "src/ast/common.h"

namespace tanlang {
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::APFloat;
using llvm::APInt;
using llvm::Type;
using llvm::IRBuilder;
using llvm::AllocaInst;
using llvm::Function;
using llvm::FunctionType;
using llvm::BasicBlock;
using llvm::verifyFunction;

ASTStatement::ASTStatement(bool is_compound, Token *token) : ASTNode(ASTType::STATEMENT,
                                                                     op_precedence[ASTType::STATEMENT],
                                                                     0,
                                                                     token) {
  _is_compound = is_compound;
}
ASTStatement::ASTStatement(Token *token) : ASTNode(ASTType::STATEMENT,
                                                   op_precedence[ASTType::STATEMENT],
                                                   0,
                                                   token) {
}

ASTProgram::ASTProgram() : ASTNode(ASTType::PROGRAM, op_precedence[ASTType::PROGRAM], 0, nullptr) {}

/**
 * \brief: parse a list of (compound) statements
 * */
void ASTProgram::nud(Parser *parser) {
  size_t n_tokens = parser->_tokens.size();
  auto *t = parser->_tokens[parser->_curr_token];
  if (t->type == TokenType::PUNCTUATION && t->value == "{") {
    auto n = parser->advance();
    n->nud(parser);
    _children.push_back(n);
    return;
  }
  while (parser->_curr_token < n_tokens) {
    auto n = std::reinterpret_pointer_cast<ASTStatement>(parser->next_statement());
    if (!n || n->_children.empty()) { break; }
    _children.push_back(n);
    ++parser->_curr_token;
  }
}

/**
 * \brief: parse a statement if _is_compound is false, otherwise parse a list of (compound) statements and add them
 *          to _children.
 * */
void ASTStatement::nud(Parser *parser) {
  size_t n_tokens = parser->_tokens.size();
  if (_is_compound) {
    while (parser->_curr_token < n_tokens) {
      auto n = parser->next_statement();
      if (!n || n->_children.empty()) { break; }
      _children.push_back(n);
      ++parser->_curr_token;
    }
  } else {
    auto n = std::reinterpret_pointer_cast<ASTStatement>(parser->next_statement());
    if (n && !n->_children.empty()) {
      *this = *n;
      ++parser->_curr_token;
    }
  }
}

Value *ASTProgram::codegen(ParserContext *parser_context) {
  // make function prototype
  Type *float_type = parser_context->_builder->getFloatTy();
  std::vector<Type *> arg_types(2, float_type);
  FunctionType *FT = FunctionType::get(float_type, arg_types, false);
  Function *F = Function::Create(FT, Function::ExternalLinkage, "main", *parser_context->_module);

  // TODO: function argument type
  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName("arg" + std::to_string(Idx++));
  }

  // function implementation
  // create a new basic block to start insertion into
  BasicBlock *main_block = BasicBlock::Create(*parser_context->_context, "entry", F);
  parser_context->_builder->SetInsertPoint(main_block);

  // TODO: create a new scope
  // Record the function arguments in the NamedValues map.
  for (auto &Arg : F->args()) {
    parser_context->add_variable(Arg.getName(), &Arg);
    // Create an alloca for this variable.
    AllocaInst *alloca = CreateEntryBlockAlloca(F, Arg.getName(), parser_context);
    // Store the initial value into the alloca.
    parser_context->_builder->CreateStore(&Arg, alloca);

    // Add arguments to variable symbol table.
    parser_context->set_variable(Arg.getName(), alloca);
  }

  for (const auto &child : _children) {
    child->codegen(parser_context);
  }
  // validate the generated code, checking for consistency
  verifyFunction(*F);
  return nullptr;
}

}
