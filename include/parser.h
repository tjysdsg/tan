#ifndef TAN_PARSER_H
#define TAN_PARSER_H
#include "ast.h"
#include "compiler_session.h"
#include "lexer.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <stack>
#include <vector>

namespace tanlang {

class ASTNode;

class Parser {
 public:
  Parser() = delete;
  ~Parser();
  explicit Parser(std::vector<Token *> tokens);

  std::shared_ptr<ASTNode> advance();
  std::shared_ptr<ASTNode> advance(TokenType type, const std::string &value);
  std::shared_ptr<ASTNode> peek();
  std::shared_ptr<ASTNode> peek(TokenType type, const std::string &value);
  std::shared_ptr<ASTNode> next_expression(int rbp = 0);
  std::shared_ptr<ASTNode> next_statement();
  std::shared_ptr<ASTNode> parse();
  [[nodiscard]] Token *get_curr_token() const;
  Value *codegen();
  virtual Error evaluate(std::unique_ptr<Module> module = nullptr) {
    UNUSED(module);
    return Error::success();
  };

  std::vector<Token *> _tokens;
  std::shared_ptr<ASTNode> _root{};
  size_t _curr_token;

 public:
  template<ASTType first_type, ASTType... types>
  std::shared_ptr<ASTNode> parse(bool strict) {
    // NOTE: strict is not used here, but used in specialized template functions defined in parser.cpp
    size_t token_index = _curr_token;
    std::shared_ptr<ASTNode> node;
    node = parse<first_type>(sizeof...(types) == 0); // if no fallback parsing, strict is true
    if (!node) { // if failed, go fallback
      _curr_token = token_index;
      if constexpr (sizeof...(types) > 0) return parse<types...>(false);
      else { // no fallback
        if (strict) {
          throw std::runtime_error("All parsing failed");
        }
        return nullptr;
      }
    }
    return node;
  }

 protected:
  CompilerSession *_compiler_session;

 public:
  [[nodiscard]] CompilerSession *get_compiler_session() const { return _compiler_session; };
};

} // namespace tanlang

#endif /* TAN_PARSER_H */
