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

struct Token;

class Parser {
public:
  Parser() = delete;
  virtual ~Parser();
  Parser(std::vector<Token *> tokens, std::string filename);

  std::shared_ptr<ASTNode> peek(size_t &index);
  std::shared_ptr<ASTNode> peek(size_t &index, TokenType type, const std::string &value);
  std::shared_ptr<ASTNode> next_expression(size_t &index, int rbp = 0);
  std::shared_ptr<ASTNode> parse();
  Value *codegen();

  bool eof(size_t index) const { return index >= _tokens.size(); }

  [[nodiscard]] Token *at(const size_t idx) const;

  [[nodiscard]] std::string get_filename() const { return _filename; }

public:
  std::shared_ptr<ASTNode> _root{};

protected:
  std::vector<Token *> _tokens;
  CompilerSession *_compiler_session;
  std::string _filename;

public:
  [[nodiscard]] CompilerSession *get_compiler_session() const { return _compiler_session; };

public:
  template<ASTType first_type, ASTType... types> std::shared_ptr<ASTNode> parse(size_t &index, bool strict) {
    /// NOTE: strict is not used here, but used in specialized template functions defined in parser.cpp
    size_t token_index = index;
    std::shared_ptr<ASTNode> node;
    node = parse<first_type>(index, sizeof...(types) == 0); // if no fallback parsing, strict is true
    if (!node) { // if failed, go fallback
      index = token_index;
      if constexpr (sizeof...(types) > 0) { return parse<types...>(index, false); }
      else { // no fallback
        if (strict) {
          throw std::runtime_error("All parsing failed");
        }
        return nullptr;
      }
    }
    return node;
  }
};

} // namespace tanlang

#endif /* TAN_PARSER_H */
