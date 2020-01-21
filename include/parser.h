#ifndef TAN_PARSER_H
#define TAN_PARSER_H

#include "lexer.h"
#include <vector>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace tanlang {

using llvm::Value;
using llvm::LLVMContext;
using llvm::IRBuilder;
using llvm::Module;
class ASTNode;

class Parser final {
 public:
  Parser() = delete;

  explicit Parser(std::vector<Token *> tokens) : _tokens(std::move(tokens)), _curr_token(0) {
    _module = new Module("shit", _context);
  }

  ~Parser();

  ASTNode *advance();
  ASTNode *peek();
  ASTNode *next_expression(int rbp = 0);
  ASTNode *parse();

  std::vector<Token *> _tokens;
  ASTNode *_root = nullptr;
  size_t _curr_token;

 public:
  LLVMContext _context = LLVMContext();
  IRBuilder<> _builder = IRBuilder<>(_context);
  Module *_module;
};
}

#endif /* TAN_PARSER_H */
