#include "token.h"
#include "src/common.h"

namespace tanlang {

bool check_typename_token(Token *token) {
  return token->type == TokenType::KEYWORD
      && is_string_in(token->value, {"int", "float", "double", "i16", "u16", "i32", "u32", "i64", "u64"});
}

bool check_terminal_token(Token *token) {
  return token->type == TokenType::PUNCTUATION && is_string_in(token->value, {";", "}", ")", ":", ",", "]"});
}

bool check_arithmetic_token(Token *token) {
  return token->type == TokenType::BOP && is_string_in(token->value, {"+", "-", "*", "/", "%"});
}

} // namespace tanlang
