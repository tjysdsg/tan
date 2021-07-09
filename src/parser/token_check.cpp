#include "token.h"
#include "src/common.h"

namespace tanlang {

bool check_typename_token(Token *token) {
  return is_string_in(token->get_value(), TYPE_NAMES);
}

static vector<str> TERMINAL_TOKENS{";", "}", ")", ":", ",", "]"};
static vector<str> ARITHMETIC_TOKENS{"+", "-", "*", "/", "%"};

bool check_terminal_token(Token *token) {
  return token->get_type() == TokenType::PUNCTUATION && is_string_in(token->get_value(), TERMINAL_TOKENS);
}

bool check_arithmetic_token(Token *token) {
  return token->get_type() == TokenType::BOP && is_string_in(token->get_value(), ARITHMETIC_TOKENS);
}

} // namespace tanlang
