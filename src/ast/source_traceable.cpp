#include "source_traceable.h"
#include "token.h"

using namespace tanlang;

size_t SourceTraceable::get_line() {
  return get_token()->l + 1;
}

size_t SourceTraceable::get_col() {
  return get_token()->c + 1;
}

Token *SourceTraceable::get_token() {
  return _token;
}

str SourceTraceable::get_token_str() {
  return _token->value;
}

void SourceTraceable::set_token(Token *token) {
  _token = token;
}

