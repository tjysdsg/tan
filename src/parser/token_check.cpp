#include "token.h"
#include "src/common.h"

namespace tanlang {

bool check_typename_token(Token *token) {
  return is_string_in(token->value, TYPE_NAMES);
}

static vector<str> TERMINAL_TOKENS{";", "}", ")", ":", ",", "]"};
static vector<str> ARITHMETIC_TOKENS{"+", "-", "*", "/", "%"};

bool check_terminal_token(Token *token) {
  return token->type == TokenType::PUNCTUATION && is_string_in(token->value, TERMINAL_TOKENS);
}

bool check_arithmetic_token(Token *token) {
  return token->type == TokenType::BOP && is_string_in(token->value, ARITHMETIC_TOKENS);
}

} // namespace tanlang
