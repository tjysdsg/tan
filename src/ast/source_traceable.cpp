#include "source_traceable.h"
#include "token.h"
#include <iostream>

using namespace tanlang;

// TODO: #43
size_t SourceTraceable::get_line() const {
  Token *tok = get_token();
  if (!tok) {
    std::cerr << "WARNING: get_token() returned nullptr\n";
    return 0;
  } else {
    return tok->get_line() + 1;
  }
}

// TODO: #43
size_t SourceTraceable::get_col() const {
  Token *tok = get_token();
  if (!tok) {
    std::cerr << "WARNING: get_token() returned nullptr\n";
    return 0;
  } else {
    return tok->get_col() + 1;
  }
}

Token *SourceTraceable::get_token() const {
  return _token;
}

str SourceTraceable::get_token_str() const {
  return _token->get_value();
}

void SourceTraceable::set_token(Token *token) {
  _token = token;
}
