#include "parser.h"
#include "src/ast/ast_statement.h"
#include "src/ast/ast_func.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_expr.h"
#include "src/ast/ast_dot.h"
#include <memory>

namespace tanlang {

Parser::Parser(std::vector<Token *> tokens) : _tokens(std::move(tokens)), _curr_token(0) {
  _compiler_session = new CompilerSession("main");
}

Parser::~Parser() {
  delete _compiler_session;
}

std::shared_ptr<ASTNode> Parser::advance() {
  auto r = peek();
  ++_curr_token;
  return r;
}

std::shared_ptr<ASTNode> Parser::advance(TokenType type, const std::string &value) {
  auto r = peek(type, value);
  ++_curr_token;
  return r;
}

std::shared_ptr<ASTNode> Parser::peek(TokenType type, const std::string &value) {
  if (_curr_token >= _tokens.size()) {
    throw std::runtime_error("Unexpected EOF"); // improve error
  }
  Token *token = _tokens[_curr_token];
  if (token->type != type || token->value != value) {
    report_code_error(token,
                      "Expect token " + value + " with type " + token_type_names[type] + ", but got "
                          + token->to_string() + " instead"
    );
  }
  return peek();
}

std::shared_ptr<ASTNode> Parser::peek() {
  if (_curr_token >= _tokens.size()) { return nullptr; }
  Token *token = _tokens[_curr_token];
  std::shared_ptr<ASTNode> node;
  if (token->value == "+" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTArithmetic>(ASTType::SUM, token);
  } else if (token->value == "-" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTArithmetic>(ASTType::SUBTRACT, token);
  } else if (token->value == "*" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTArithmetic>(ASTType::MULTIPLY, token);
  } else if (token->value == "/" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTArithmetic>(ASTType::DIVIDE, token);
  } else if (token->value == "=" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTAssignment>(token);
  } else if (token->value == "!" && token->type == TokenType::UOP) {
    node = std::make_shared<ASTLogicalNot>(token);
  } else if (token->value == "~" && token->type == TokenType::UOP) {
    node = std::make_shared<ASTBinaryNot>(token);
  } else if (token->value == "struct" && token->type == TokenType::KEYWORD) {
    node = std::make_shared<ASTStruct>(token);
  } else if (token->type == TokenType::RELOP) {
    if (token->value == ">") {
      node = std::make_shared<ASTCompare>(ASTType::GT, token);
    } else if (token->value == ">=") {
      node = std::make_shared<ASTCompare>(ASTType::GE, token);
    } else if (token->value == "<") {
      node = std::make_shared<ASTCompare>(ASTType::LT, token);
    } else if (token->value == "<=") {
      node = std::make_shared<ASTCompare>(ASTType::LE, token);
    }
  } else if (token->type == TokenType::INT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, false, token);
  } else if (token->type == TokenType::FLOAT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, true, token);
  } else if (token->type == TokenType::STRING) {
    node = std::make_shared<ASTStringLiteral>(token->value, token);
  } else if (token->type == TokenType::ID) {
    size_t token_index = _curr_token;
    node = parse<ASTType::FUNC_CALL, ASTType::ID>(false);
    _curr_token = token_index;
  } else if (token->type == TokenType::PUNCTUATION && token->value == "(") {
    node = std::make_shared<ASTParenthesis>(token);
  } else if (token->type == TokenType::KEYWORD && token->value == "var") {
    node = std::make_shared<ASTVarDecl>(token);
  } else if (token->type == TokenType::KEYWORD && token->value == "fn") {
    node = std::make_shared<ASTFunction>(token);
  } else if (token->type == TokenType::KEYWORD && token->value == "if") {
    node = std::make_shared<ASTIf>(token);
  } else if (token->type == TokenType::KEYWORD && token->value == "else") {
    node = std::make_shared<ASTElse>(token);
  } else if (token->type == TokenType::KEYWORD && token->value == "return") {
    node = std::make_shared<ASTReturn>(token);
  } else if (token->type == TokenType::BOP && token->value == ".") { // member access
    node = std::make_shared<ASTDot>(token);
  } else if (check_typename_token(token)) { // types
    node = std::make_shared<ASTTy>(token);
  } else if (token->type == TokenType::PUNCTUATION && token->value == "{") {
    node = std::make_shared<ASTStatement>(true, token);
  } else if (check_terminal_token(token)) {
    return nullptr; // FIXME: nullptr represent a terminal symbol, like statements ending with a semicolon
  } else {
    report_code_error(token, "Unknown token " + token->to_string());
  }
  return node;
}

std::shared_ptr<ASTNode> Parser::next_expression(int rbp) {
  std::shared_ptr<ASTNode> node = advance();
  if (!node) {
    return nullptr;
  }
  auto n = node;
  n->nud(this);
  auto left = n;
  node = peek();
  if (!node) {
    return left;
  }
  while (rbp < node->_lbp) {
    node = peek();
    n = node;
    ++_curr_token;
    n->led(left, this);
    left = n;
    node = peek();
    if (!node) { break; };
  }
  return left;
}

std::shared_ptr<ASTNode> Parser::parse() {
  _root = std::make_shared<ASTProgram>();
  _root->nud(this);
  return _root;
}

Token *Parser::get_curr_token() const {
  return _tokens[_curr_token];
}

Value *Parser::codegen() {
  return _root->codegen(_compiler_session);
}

void Parser::dump() const {
  get_compiler_session()->get_module()->print(llvm::outs(), nullptr);
}

Error Parser::evaluate(std::unique_ptr<Module> module) {
  UNUSED(module);
  return Error::success();
}

#define TRY_NUD(node, strict)                                                  \
  do {                                                                         \
    try {                                                                      \
      node->nud(this);                                                         \
    } catch (const std::runtime_error &e) {                                    \
      if (strict)                                                              \
        throw e;                                                               \
      return nullptr;                                                          \
    }                                                                          \
  } while (false)

template<> std::shared_ptr<ASTNode> Parser::parse<ASTType::STATEMENT>(bool strict) {
  auto *token = get_curr_token();
  std::shared_ptr<ASTNode> node = std::make_shared<ASTStatement>(token);
  TRY_NUD(node, strict);
  return node;
}

template<> std::shared_ptr<ASTNode> Parser::parse<ASTType::ID>(bool strict) {
  auto *token = get_curr_token();
  std::shared_ptr<ASTNode> node = std::make_shared<ASTIdentifier>(token->value, token);
  ++_curr_token;
  TRY_NUD(node, strict);
  return node;
}

template<> std::shared_ptr<ASTNode> Parser::parse<ASTType::FUNC_CALL>(bool strict) {
  auto *token = get_curr_token();
  std::shared_ptr<ASTNode> node = std::make_shared<ASTFunctionCall>(token->value, token);
  ++_curr_token;
  TRY_NUD(node, strict);
  return node;
}

template<> std::shared_ptr<ASTNode> Parser::parse<ASTType::TY>(bool strict) {
  auto *token = get_curr_token();
  std::shared_ptr<ASTNode> node = std::make_shared<ASTTy>(token);
  TRY_NUD(node, strict);
  return node;
}

} // namespace tanlang
