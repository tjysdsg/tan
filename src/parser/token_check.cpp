#include "token.h"
#include "src/common.h"

namespace tanlang {

bool check_typename_token(Token *token) {
  return is_string_in(token->value, TYPE_NAMES);
}

bool check_terminal_token(Token *token) {
  return token->type == TokenType::PUNCTUATION && is_string_in(token->value, {";", "}", ")", ":", ",", "]"});
}

bool check_arithmetic_token(Token *token) {
  return token->type == TokenType::BOP && is_string_in(token->value, {"+", "-", "*", "/", "%"});
}

} // namespace tanlang
