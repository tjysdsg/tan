#ifndef TAN_PARSER_H
#define TAN_PARSER_H
#include "lexer.h"
#include "ast.h"
#include "compiler_session.h"
#include <vector>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <stack>

namespace tanlang {
using llvm::Value;
using llvm::LLVMContext;
using llvm::IRBuilder;
using llvm::BasicBlock;
using llvm::Module;

class ASTNode;
class Parser {
 public:
  Parser() = delete;
  ~Parser() = default;
  explicit Parser(std::vector<Token *> tokens);

  std::shared_ptr<ASTNode> advance();
  std::shared_ptr<ASTNode> advance(TokenType type, const std::string &value);
  std::shared_ptr<ASTNode> peek();
  std::shared_ptr<ASTNode> peek(TokenType type, const std::string &value);
  std::shared_ptr<ASTNode> next_expression(int rbp = 0);
  std::shared_ptr<ASTNode> next_node();
  std::shared_ptr<ASTNode> next_statement();
  std::shared_ptr<ASTNode> parse();
  [[nodiscard]] Token *get_curr_token() const;

  std::vector<Token *> _tokens;
  std::shared_ptr<ASTNode> _root{};
  size_t _curr_token;

 public:
  CompilerSession *_parser_context;
};
}

#endif /* TAN_PARSER_H */
