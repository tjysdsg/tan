#include "parser.h"
#include "src/ast/ast_array.h"
#include "src/ast/ast_ampersand.h"
#include "src/ast/ast_cast.h"
#include "src/ast/ast_import.h"
#include "src/ast/ast_member_access.h"
#include "src/ast/ast_string_literal.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_statement.h"
#include "src/ast/ast_loop.h"
#include "src/parser/token_check.h"
#include "src/parser/parser.hpp"
#include "src/ast/ast_parenthesis.h"
#include "src/ast/ast_var_decl.h"
#include "src/ast/ast_not.h"
#include "src/ast/ast_arithmetic.h"
#include "src/ast/ast_compare.h"
#include "src/ast/ast_assignment.h"
#include "src/ast/ast_number_literal.h"
#include "src/ast/ast_control_flow.h"
#include "src/ast/ast_return.h"
#include "src/ast/ast_struct.h"
#include "src/ast/ast_program.h"
#include "src/ast/ast_char_literal.h"
#include "src/common.h"
#include "intrinsic.h"
#include "token.h"
#include <memory>

namespace tanlang {

Parser::Parser(std::vector<Token *> tokens, const std::string &filename, CompilerSession *cs)
    : _tokens(std::move(tokens)), _filename(filename), _cs(cs) {}

ASTNodePtr Parser::peek(size_t &index, TokenType type, const std::string &value) {
  if (index >= _tokens.size()) {
    throw std::runtime_error("Unexpected EOF");
  }
  Token *token = _tokens[index];
  if (token->type != type || token->value != value) {
    report_code_error(token,
        "Expect token " + value + " with type " + token_type_names[type] + ", but got " + token->to_string()
            + " instead");
  }
  return peek(index);
}

static ASTNodePtr peek_keyword(Token *token, size_t &index) {
  switch_str(token->value)
  case_str0("var") return std::make_shared<ASTVarDecl>(token, index); //
  case_str_or3("fn", "pub", "extern") return std::make_shared<ASTFunction>(token, index); //
  case_str("import") return std::make_shared<ASTImport>(token, index); //
  case_str("if") return std::make_shared<ASTIf>(token, index); //
  case_str("else") return std::make_shared<ASTElse>(token, index); //
  case_str("return") return std::make_shared<ASTReturn>(token, index); //
  case_str_or2("while", "for") return std::make_shared<ASTLoop>(token, index); //
  case_str("struct") return std::make_shared<ASTStruct>(token, index); //
  case_str_or2("break", "continue") return std::make_shared<ASTBreakContinue>(token, index); //
  case_str("as") return std::make_shared<ASTCast>(token, index); //
  case_default report_code_error(token, "Keyword not implemented: " + token->to_string()); //
  end_switch
}

ASTNodePtr Parser::peek(size_t &index) {
  if (index >= _tokens.size()) { return nullptr; }
  Token *token = _tokens[index];
  /// skip comments
  while (token->type == TokenType::COMMENTS) {
    ++index;
    token = _tokens[index];
  }
  ASTNodePtr node;
  if (token->value == "@") { /// intrinsics
    node = std::make_shared<Intrinsic>(token, index);
  } else if (token->value == "=" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTAssignment>(token, index);
  } else if (token->value == "!" || token->value == "~") {
    node = std::make_shared<ASTNot>(token, index);
  } else if (token->value == "[") {
    auto prev = this->at(index - 1);
    if (prev->type != TokenType::ID && prev->value != "]" && prev->value != ")") {
      /// array literal if there is no identifier, "]", or ")" before
      node = std::make_shared<ASTArrayLiteral>(token, index);
    } else {
      /// otherwise bracket access
      node = std::make_shared<ASTMemberAccess>(token, index);
    }
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
    } else if (token->value == "!=") {
      node = std::make_shared<ASTCompare>(ASTType::NE, token, index);
    }
  } else if (token->type == TokenType::INT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, false, token, index);
  } else if (token->type == TokenType::FLOAT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, true, token, index);
  } else if (token->type == TokenType::STRING) {
    node = std::make_shared<ASTStringLiteral>(token, index);
  } else if (token->type == TokenType::CHAR) {
    node = std::make_shared<ASTCharLiteral>(token, index);
  } else if (token->type == TokenType::ID) {
    Token *next = _tokens[index + 1];
    if (next->value == "(") { node = std::make_shared<ASTFunctionCall>(token, index); }
    else { node = std::make_shared<ASTIdentifier>(token, index); }
  } else if (token->type == TokenType::PUNCTUATION && token->value == "(") {
    node = std::make_shared<ASTParenthesis>(token, index);
  } else if (token->type == TokenType::KEYWORD) { /// keywords
    node = peek_keyword(token, index);
  } else if (token->type == TokenType::BOP && token->value == ".") { /// member access
    node = std::make_shared<ASTMemberAccess>(token, index);
  } else if (check_typename_token(token)) { /// types
    node = std::make_shared<ASTTy>(token, index);
  } else if (token->value == "&") {
    node = std::make_shared<ASTAmpersand>(token, index);
  } else if (token->type == TokenType::PUNCTUATION && token->value == "{") {
    node = std::make_shared<ASTStatement>(true, token, index);
  } else if (token->type == TokenType::BOP && check_arithmetic_token(token)) {
    node = std::make_shared<ASTArithmetic>(token, index);
  } else if (check_terminal_token(token)) { /// this MUST be the last thing to check
    return nullptr;
  } else {
    report_code_error(token, "Unknown token " + token->to_string());
  }
  return node;
}

ASTNodePtr Parser::next_expression(size_t &index, int rbp) {
  ASTNodePtr node = peek(index);
  ++index;
  if (!node) { return nullptr; }
  auto n = node;
  index = n->parse(this, _cs);
  auto left = n;
  node = peek(index);
  if (!node) { return left; }
  while (rbp < node->_lbp) {
    node = peek(index);
    n = node;
    index = n->parse(left, this, _cs);
    left = n;
    node = peek(index);
    if (!node) { break; }
  }
  return left;
}

ASTNodePtr Parser::parse() {
  _root = std::make_shared<ASTProgram>();
  (void) _root->parse(this, _cs); /// fix the [[nodiscard]] warning
  return _root;
}

Token *Parser::at(const size_t idx) const {
  if (this->eof(idx)) { throw std::runtime_error("Unexpected EOF"); }
  return _tokens[idx];
}

std::string Parser::get_filename() const { return _filename; }

bool Parser::eof(size_t index) const { return index >= _tokens.size(); }

std::shared_ptr<ASTNode> Parser::get_ast() const { return _root; }

} // namespace tanlang
