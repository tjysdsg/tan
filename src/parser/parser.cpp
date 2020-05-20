#include "parser.h"
#include "base.h"
#include "compiler_session.h"
#include "src/analysis/type_system.h"
#include "src/analysis/analysis.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_ty.h"
#include "src/common.h"
#include "intrinsic.h"
#include "token.h"
#include <memory>

namespace tanlang {

Parser::Parser(vector<Token *> tokens, const str &filename, CompilerSession *cs)
    : _tokens(std::move(tokens)), _filename(filename), _cs(cs) {}

ASTNodePtr Parser::peek(size_t &index, TokenType type, const str &value) {
  if (index >= _tokens.size()) { report_error(_filename, _tokens.back(), "Unexpected EOF"); }
  Token *token = _tokens[index];
  if (token->type != type || token->value != value) {
    report_error(_filename, token, "Expect '" + value + "', but got '" + token->value + "' instead");
  }
  return peek(index);
}

static ASTNodePtr peek_keyword(Token *token, size_t &index) {
  ASTNodePtr ret = nullptr;
  switch (hashed_string{token->value.c_str()}) {
    case "var"_hs:
      ret = ast_create_var_decl();
    case "enum"_hs:
      ret = ast_create_enum();
    case "fn"_hs:
    case "pub"_hs:
    case "extern"_hs:
      ret = ast_create_func_decl();
    case "import"_hs:
      ret = ast_create_import();
    case "if"_hs:
      ret = ast_create_if();
    case "else"_hs:
      ret = ast_create_else();
    case "return"_hs:
      ret = ast_create_return();
    case "while"_hs:
    case "for"_hs:
      ret = ast_create_loop();
    case "struct"_hs:
      ret = ast_create_struct_decl();
    case "break"_hs:
    case "continue"_hs:
      ret = ast_create_break_or_continue();
    case "as"_hs:
      ret = ast_create_cast();
    default:
      return nullptr;
  }
  ret->_token = token;
  ret->_start_index = index;
  return ret;
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
    node = ast_create_assignment();
  } else if (token->value == "!" || token->value == "~") {
    node = ast_create_not();
  } else if (token->value == "[") {
    auto prev = this->at(index - 1);
    if (prev->type != TokenType::ID && prev->value != "]" && prev->value != ")") {
      /// array literal if there is no identifier, "]", or ")" before
      node = ast_create_array_literal();
    } else {
      /// otherwise bracket access
      node = ast_create_member_access();
    }
  } else if (token->type == TokenType::RELOP) { /// comparisons
    node = ast_create_comparison(token->value);
  } else if (token->type == TokenType::INT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, false, token, index);
  } else if (token->type == TokenType::FLOAT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, true, token, index);
  } else if (token->type == TokenType::STRING) { /// string literal
    node = ast_create_string_literal(token->value);
  } else if (token->type == TokenType::CHAR) { /// char literal
    node = std::make_shared<ASTCharLiteral>(token, index);
  } else if (token->type == TokenType::ID) {
    Token *next = _tokens[index + 1];
    if (next->value == "(") { node = std::make_shared<ASTFunctionCall>(token, index); }
    else { node = ast_create_identifier(token->value); }
  } else if (token->type == TokenType::PUNCTUATION && token->value == "(") {
    node = std::make_shared<ASTParenthesis>(token, index);
  } else if (token->type == TokenType::KEYWORD) { /// keywords
    node = peek_keyword(token, index);
    if (!node) { report_error(_filename, token, "Keyword not implemented: " + token->to_string()); }
  } else if (token->type == TokenType::BOP && token->value == ".") { /// member access
    node = std::make_shared<ASTMemberAccess>(token, index);
  } else if (check_typename_token(token)) { /// types
    node = ast_create_ty();
  } else if (token->value == "&") {
    node = std::make_shared<ASTAmpersand>(token, index);
  } else if (token->type == TokenType::PUNCTUATION && token->value == "{") { /// statement(s)
    node = ast_create_statement();
  } else if (token->type == TokenType::BOP && check_arithmetic_token(token)) { /// arithmetic operators
    node = ast_create_arithmetic(token->value);
  } else if (check_terminal_token(token)) { /// this MUST be the last thing to check
    return nullptr;
  } else {
    report_error(_filename, token, "Unknown token " + token->to_string());
  }
  node->_token = token;
  node->_start_index = node->_end_index = index;
  return node;
}

ASTNodePtr Parser::next_expression(size_t &index, int rbp) {
  ASTNodePtr node = peek(index);
  ++index;
  if (!node) { return nullptr; }
  auto n = node;
  index = parse_node(n);
  auto left = n;
  node = peek(index);
  if (!node) { return left; }
  while (rbp < node->_lbp) {
    node = peek(index);
    n = node;
    index = parse_node(left, n);
    left = n;
    node = peek(index);
    if (!node) { break; }
  }
  return left;
}

size_t Parser::parse_node(ASTNodePtr p) {
  p->_end_index = p->_start_index;
  switch (p->_type) {
    case ASTType::PROGRAM:
      while (!eof(p->_end_index)) {
        auto stmt = ast_create_statement();
        stmt->_token = at(p->_end_index);
        stmt->_start_index = p->_end_index;
        p->_end_index = parse_node(stmt);
        p->_children.push_back(stmt);
      }
      break;
    case ASTType::STATEMENT:
      if (at(p->_end_index)->value == "{") { /// compound statement
        ++p->_end_index; /// skip "{"
        while (!eof(p->_end_index)) {
          auto node = peek(p->_end_index);
          while (node) { /// stops at a terminal token
            p->_children.push_back(next_expression(p->_end_index, 0));
            node = peek(p->_end_index);
          }
          if (at(p->_end_index)->value == "}") {
            ++p->_end_index; /// skip "}"
            break;
          }
          ++p->_end_index;
        }
      } else { /// single statement
        auto node = peek(p->_end_index);
        while (node) { /// stops at a terminal token
          p->_children.push_back(next_expression(p->_end_index, 0));
          node = peek(p->_end_index);
        }
        ++p->_end_index; /// skip ';'
      }
      break;
    case ASTType::VAR_DECL:
      ++p->_end_index; /// skip 'var'
    case ASTType::ARG_DECL: {
      /// var name
      auto name_token = at(p->_end_index);
      p->_name = name_token->value;
      ++p->_end_index;
      if (at(p->_end_index)->value == ":") {
        ++p->_end_index;
        /// type
        auto ty = ast_create_ty();
        ty->_token = at(p->_end_index);
        ty->_end_index = ty->_start_index = p->_end_index;
        ty->_is_lvalue = true;
        p->_end_index = parse_node(ty);
        p->_ty = ty;
      } else { p->_ty = nullptr; }
      if (p->_type == ASTType::VAR_DECL) { _cs->add(p->_name, p); }
      break;
    }
    case ASTType::SUM:
    case ASTType::SUBTRACT: { /// unary '+' or '-'
      ++p->_end_index; /// skip "-" or "+"
      /// higher precedence than infix plus/minus
      p->_rbp = PREC_UNARY;
      auto rhs = next_expression(p->_end_index, p->_rbp);
      if (!rhs) { error(p->_end_index, "Invalid operand"); }
      p->_children.push_back(rhs);
      break;
    }
    case ASTType::ARRAY_LITERAL: {
      ++p->_end_index; /// skip '['
      if (at(p->_end_index)->value == "]") { error(p->_end_index, "Empty array"); }
      ASTType element_type = ASTType::INVALID;
      while (!eof(p->_end_index)) {
        if (at(p->_end_index)->value == ",") {
          ++p->_end_index;
          continue;
        } else if (at(p->_end_index)->value == "]") {
          ++p->_end_index;
          break;
        }
        auto node = peek(p->_end_index);
        if (!node) { error(p->_end_index, "Unexpected token"); }
        /// check whether element types are the same
        if (element_type == ASTType::INVALID) { element_type = node->_type; }
        else {
          if (element_type != node->_type) {
            error(p->_end_index, "All elements in an array must have the same type");
          }
        }
        if (is_ast_type_in(node->_type, TypeSystem::LiteralTypes)) {
          if (node->_type == ASTType::ARRAY_LITERAL) { ++p->_end_index; }
          p->_end_index = parse_node(node);
          p->_children.push_back(node);
        } else { error(p->_end_index, "Expect literals"); }
      }

      vector<ASTNodePtr> sub_tys{};
      sub_tys.reserve(p->_children.size());
      std::for_each(p->_children.begin(),
          p->_children.end(),
          [&sub_tys](const ASTNodePtr &e) { sub_tys.push_back(get_ty(e)); });
      p->_ty = create_ty(Ty::ARRAY, sub_tys);
      break;
    }
    case ASTType::ID:
    case ASTType::NUM_LITERAL:
    case ASTType::STRING_LITERAL: /// trivially parsed ASTs
      ++p->_end_index;
      break;
    default:
      break;
  }
  return p->_end_index;
}

size_t Parser::parse_node(ASTNodePtr left, ASTNodePtr p) {
  p->_end_index = p->_start_index;
  switch (p->_type) {
    case ASTType::GT:
    case ASTType::GE:
    case ASTType::LT:
    case ASTType::LE:
    case ASTType::EQ:
    case ASTType::NE:
    case ASTType::SUM:
    case ASTType::SUBTRACT:
    case ASTType::MULTIPLY:
    case ASTType::DIVIDE:
    case ASTType::MOD: {
      ++p->_end_index; /// skip operator
      p->_children.push_back(left); /// lhs
      auto n = next_expression(p->_end_index, p->_lbp);
      if (!n) { error(p->_end_index, "Invalid operand"); }
      p->_children.push_back(n);
    }
    default:
      break;
  }
  return p->_end_index;
}

ASTNodePtr Parser::parse() {
  _root = ast_create_program();
  parse_node(_root);
  return _root;
}

Token *Parser::at(const size_t idx) const {
  if (this->eof(idx)) { report_error(_filename, _tokens.back(), "Unexpected EOF"); }
  return _tokens[idx];
}

str Parser::get_filename() const { return _filename; }

bool Parser::eof(size_t index) const { return index >= _tokens.size(); }

void Parser::error(const str &error_message) {
  if (_cs && _cs->_current_token) {
    report_error(get_filename(), _cs->_current_token, error_message);
  } else { report_error(error_message); }
}

void Parser::error(size_t i, const str &error_message) { report_error(get_filename(), at(i), error_message); }

} // namespace tanlang
