#ifndef TAN_PARSER_H
#define TAN_PARSER_H
#include "lexer.h"
#include "ast.h"
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

struct ParserContext {
  std::unique_ptr<LLVMContext> _context;
  std::unique_ptr<IRBuilder<>> _builder;
  std::unique_ptr<Module> _module;
  std::stack<std::shared_ptr<Scope>> _scope;

  ParserContext &operator=(const ParserContext &) = delete;
  ParserContext(const ParserContext &) = delete;
  ParserContext() {
    _context = std::make_unique<LLVMContext>();
    _builder = std::make_unique<IRBuilder<>>(*_context);
    _module = std::make_unique<Module>("main", *_context);
    _scope = std::stack<std::shared_ptr<Scope>>();
    _scope.push(std::make_shared<Scope>()); // outer-est scope
  }

  explicit ParserContext(const std::string &module_name) {
    _context = std::make_unique<LLVMContext>();
    _builder = std::make_unique<IRBuilder<>>(*_context);
    _module = std::make_unique<Module>(module_name, *_context);
    _scope = std::stack<std::shared_ptr<Scope>>();
    _scope.push(std::make_shared<Scope>()); // outer-est scope
  }

  std::shared_ptr<Scope> get_current_scope() {
    return _scope.top();
  }

  std::shared_ptr<Scope> push_scope() {
    auto r = std::make_shared<Scope>();
    _scope.push(r);
    return r;
  }

  std::shared_ptr<Scope> pop_scope() {
    // FIXME check null
    auto r = _scope.top();
    _scope.pop();
    return r;
  }

  void add_variable(const std::string &name, Value *value) {
    get_current_scope()->_named_variables.insert(std::make_pair(name, value));
  }

  void set_variable(const std::string &name, Value *value) {
    get_current_scope()->_named_variables[name] = value;
  }

  Value *get_variable(const std::string &name) {
    return get_current_scope()->_named_variables[name];
  }
};

class ASTNode;
class Parser final {
 public:
  Parser() = delete;
  ~Parser() = default;
  explicit Parser(std::vector<Token *> tokens) : _tokens(std::move(tokens)), _curr_token(0) {
    _parser_context = new ParserContext("main");
  }

  std::shared_ptr<ASTNode> advance();
  std::shared_ptr<ASTNode> advance(TokenType type, const std::string &value);
  std::shared_ptr<ASTNode> peek();
  std::shared_ptr<ASTNode> peek(TokenType type, const std::string &value);
  std::shared_ptr<ASTNode> next_expression(int rbp = 0);
  std::shared_ptr<ASTNode> next_node();
  std::shared_ptr<ASTNode> next_statement();
  std::shared_ptr<ASTNode> parse();

  [[nodiscard]] Token *get_curr_token() const {
    return _tokens[_curr_token];
  }

  std::vector<Token *> _tokens;
  std::shared_ptr<ASTNode> _root{};
  size_t _curr_token;

 public:
  ParserContext *_parser_context;
};
}

#endif /* TAN_PARSER_H */
