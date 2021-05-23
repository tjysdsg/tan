#include "source_traceable.h"
#include "token.h"

using namespace tanlang;

size_t SourceTraceable::get_line() const {
  return get_token()->l + 1;
}

size_t SourceTraceable::get_col() const {
  return get_token()->c + 1;
}

Token *SourceTraceable::get_token() const {
  return _token;
}

str SourceTraceable::get_token_str() const {
  return _token->value;
}

void SourceTraceable::set_token(Token *token) {
  _token = token;
}

