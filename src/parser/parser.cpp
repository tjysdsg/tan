#include "parser.h"
#include "src/ast/ast_array.h"
#include "src/ast/ast_member_access.h"
#include "src/ast/ast_expr.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_statement.h"
#include "src/parser/token_check.h"
#include "src/parser/parser.hpp"
#include "intrinsic.h"
#include <memory>

namespace tanlang {

Parser::Parser(std::vector<Token *> tokens, std::string filename) : _tokens(std::move(tokens)), _filename(filename) {
  _compiler_session = new CompilerSession(filename);
}

Parser::~Parser() { delete _compiler_session; }

std::shared_ptr<ASTNode> Parser::peek(size_t &index, TokenType type, const std::string &value) {
  if (index >= _tokens.size()) {
    throw std::runtime_error("Unexpected EOF");
  }
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
  if (index >= _tokens.size()) {
    return nullptr;
  }
  Token *token = _tokens[index];
  /// skip comments
  while (token->type == TokenType::COMMENTS) {
    ++index;
    token = _tokens[index];
  }
  std::shared_ptr<ASTNode> node;
  if (Intrinsic::intrinsics.find(token->value) != Intrinsic::intrinsics.end()) { /// intrinsics
    node = std::make_shared<Intrinsic>(token, index);
  } else if (token->value == "+" && token->type == TokenType::BOP) {
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
    auto prev = this->at(index - 1);
    if (prev->type != TokenType::ID && prev->value != "]" && prev->value != ")") {
      /// array literal if there is no identifier, "]", or ")" before
      node = std::make_shared<ASTArrayLiteral>(token, index);
    } else {
      /// otherwise bracket access
      node = std::make_shared<ASTMemberAccess>(token, index);
    }
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
    } else if (token->value == "==") {
      node = std::make_shared<ASTCompare>(ASTType::EQ, token, index);
    }
    // TODO: !=
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
    node = std::make_shared<ASTMemberAccess>(token, index);
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
  if (!node) {
    return nullptr;
  }
  auto n = node;
  index = n->parse(this);
  auto left = n;
  node = peek(index);
  if (!node) {
    return left;
  }
  while (rbp < node->_lbp) {
    node = peek(index);
    n = node;
    index = n->parse(left, this);
    left = n;
    node = peek(index);
    if (!node) {
      break;
    };
  }
  return left;
}

std::shared_ptr<ASTNode> Parser::parse() {
  _root = std::make_shared<ASTProgram>();
  (void) _root->parse(this); /// fix the [[nodiscard]] warning
  return _root;
}

Value *Parser::codegen() {
  Intrinsic::InitCodegen(_compiler_session);
  return _root->codegen(_compiler_session);
}

void Parser::dump() const {
  get_compiler_session()->get_module()->print(llvm::outs(), nullptr);
}

Token *Parser::at(const size_t idx) const {
  if (this->eof(idx)) {
    throw std::runtime_error("Unexpected EOF"); // TODO: better error
  }
  return _tokens[idx];
}

} // namespace tanlang
