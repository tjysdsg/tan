#include "token.h"
#include "src/parser/token_check.h"

namespace tanlang {

bool check_typename_token(Token *token) {
  return token->type == TokenType::KEYWORD
      && (token->value == "int" || token->value == "float" || token->value == "double"
          || token->value == "i16" || token->value == "u16"
          || token->value == "i32" || token->value == "u32"
          || token->value == "i64" || token->value == "u64"
      );
}

bool check_terminal_token(Token *token) {
  return token->type == TokenType::PUNCTUATION
      && (token->value == ";" || token->value == "}" || token->value == ")" || token->value == ":"
          || token->value == ",");
}

}
