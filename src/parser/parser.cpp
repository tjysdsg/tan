#include "parser.h"
#include "src/ast/ast_array.h"
#include "src/ast/ast_dot.h"
#include "src/ast/ast_expr.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_statement.h"
#include "src/parser/token_check.h"
#include <memory>

namespace tanlang {

Parser::Parser(std::vector<Token *> tokens) : _tokens(std::move(tokens)) {
  _compiler_session = new CompilerSession("main"); // FIXME: module name
}

Parser::~Parser() { delete _compiler_session; }

std::shared_ptr<ASTNode> Parser::peek(size_t &index, TokenType type, const std::string &value) {
  if (index >= _tokens.size()) { throw std::runtime_error("Unexpected EOF"); }
  Token *token = _tokens[index];
  if (token->type != type || token->value != value) {
    report_code_error(token,
                      "Expect token " + value + " with type " + token_type_names[type] + ", but got "
                          + token->to_string() + " instead"
    );
  }
  return peek(index);
}

std::shared_ptr<ASTNode> Parser::peek(size_t &index) {
  if (index >= _tokens.size()) { return nullptr; }
  Token *token = _tokens[index];
  /// skip comments
  while (token->type == TokenType::COMMENTS) {
    ++index;
    token = _tokens[index];
  }
  std::shared_ptr<ASTNode> node;
  if (token->value == "+" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTArithmetic>(ASTType::SUM, token, index);
  } else if (token->value == "-" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTArithmetic>(ASTType::SUBTRACT, token, index);
  } else if (token->value == "*" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTArithmetic>(ASTType::MULTIPLY, token, index);
  } else if (token->value == "/" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTArithmetic>(ASTType::DIVIDE, token, index);
  } else if (token->value == "=" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTAssignment>(token, index);
  } else if (token->value == "!" && token->type == TokenType::UOP) {
    node = std::make_shared<ASTLogicalNot>(token, index);
  } else if (token->value == "~" && token->type == TokenType::UOP) {
    node = std::make_shared<ASTBinaryNot>(token, index);
  } else if (token->value == "[") {
    node = std::make_shared<ASTArrayLiteral>(token, index);
  } else if (token->value == "struct" && token->type == TokenType::KEYWORD) {
    node = std::make_shared<ASTStruct>(token, index);
  } else if (token->type == TokenType::RELOP) {
    if (token->value == ">") {
      node = std::make_shared<ASTCompare>(ASTType::GT, token, index);
    } else if (token->value == ">=") {
      node = std::make_shared<ASTCompare>(ASTType::GE, token, index);
    } else if (token->value == "<") {
      node = std::make_shared<ASTCompare>(ASTType::LT, token, index);
    } else if (token->value == "<=") {
      node = std::make_shared<ASTCompare>(ASTType::LE, token, index);
    }
  } else if (token->type == TokenType::INT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, false, token, index);
  } else if (token->type == TokenType::FLOAT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, true, token, index);
  } else if (token->type == TokenType::STRING) {
    node = std::make_shared<ASTStringLiteral>(token, index);
  } else if (token->type == TokenType::ID) {
    /// `parse` will modify the first argument, which we don't want
    size_t token_index = index;
    node = parse<ASTType::FUNC_CALL, ASTType::ID>(token_index, false);
  } else if (token->type == TokenType::PUNCTUATION && token->value == "(") {
    node = std::make_shared<ASTParenthesis>(token, index);
  } else if (token->type == TokenType::KEYWORD && token->value == "var") {
    node = std::make_shared<ASTVarDecl>(token, index);
  } else if (token->type == TokenType::KEYWORD && token->value == "fn") {
    node = std::make_shared<ASTFunction>(token, index);
  } else if (token->type == TokenType::KEYWORD && token->value == "if") {
    node = std::make_shared<ASTIf>(token, index);
  } else if (token->type == TokenType::KEYWORD && token->value == "else") {
    node = std::make_shared<ASTElse>(token, index);
  } else if (token->type == TokenType::KEYWORD && token->value == "return") {
    node = std::make_shared<ASTReturn>(token, index);
  } else if (token->type == TokenType::BOP && token->value == ".") { // member access
    node = std::make_shared<ASTDot>(token, index);
  } else if (check_typename_token(token)) { // types
    node = std::make_shared<ASTTy>(token, index);
  } else if (token->type == TokenType::PUNCTUATION && token->value == "{") {
    node = std::make_shared<ASTStatement>(true, token, index);
  } else if (check_terminal_token(token)) { /// this MUST be the last thing to check
    return nullptr;
  } else {
    report_code_error(token, "Unknown token " + token->to_string());
  }
  return node;
}

std::shared_ptr<ASTNode> Parser::next_expression(size_t &index, int rbp) {
  std::shared_ptr<ASTNode> node = peek(index);
  ++index;
  if (!node) { return nullptr; }
  auto n = node;
  index = n->nud(this);
  auto left = n;
  node = peek(index);
  if (!node) { return left; }
  while (rbp < node->_lbp) {
    node = peek(index);
    n = node;
    index = n->led(left, this);
    left = n;
    node = peek(index);
    if (!node) { break; };
  }
  return left;
}

std::shared_ptr<ASTNode> Parser::parse() {
  _root = std::make_shared<ASTProgram>();
  (void) _root->nud(this); /// fix the [[nodiscard]] warning
  return _root;
}

Value *Parser::codegen() { return _root->codegen(_compiler_session); }

void Parser::dump() const {
  get_compiler_session()->get_module()->print(llvm::outs(), nullptr);
}

Error Parser::evaluate(std::unique_ptr<Module> module) {
  UNUSED(module);
  return Error::success();
}

Token *Parser::at(const size_t idx) const {
  if (this->eof(idx)) {
    throw std::runtime_error("Unexpected EOF"); // TODO: better error
  }
  return _tokens[idx];
}

#define TRY_NUD(node, strict)                                                  \
  do {                                                                         \
    try {                                                                      \
      index = node->nud(this);                                                 \
    } catch (const std::runtime_error &e) {                                    \
      if (strict)                                                              \
        throw e;                                                               \
      return nullptr;                                                          \
    }                                                                          \
  } while (false)

template<> std::shared_ptr<ASTNode> Parser::parse<ASTType::STATEMENT>(size_t &index, bool strict) {
  auto *token = this->at(index);
  std::shared_ptr<ASTNode> node = std::make_shared<ASTStatement>(token, index);
  TRY_NUD(node, strict);
  return node;
}

template<> std::shared_ptr<ASTNode> Parser::parse<ASTType::ID>(size_t &index, bool strict) {
  auto *token = this->at(index);
  std::shared_ptr<ASTNode> node = std::make_shared<ASTIdentifier>(token, index);
  TRY_NUD(node, strict);
  return node;
}

template<> std::shared_ptr<ASTNode> Parser::parse<ASTType::FUNC_CALL>(size_t &index, bool strict) {
  auto *token = this->at(index);
  std::shared_ptr<ASTNode> node = std::make_shared<ASTFunctionCall>(token, index);
  TRY_NUD(node, strict);
  return node;
}

template<> std::shared_ptr<ASTNode> Parser::parse<ASTType::TY>(size_t &index, bool strict) {
  auto *token = this->at(index);
  std::shared_ptr<ASTNode> node = std::make_shared<ASTTy>(token, index);
  TRY_NUD(node, strict);
  return node;
}

} // namespace tanlang
