#include "source_file/source_manager.h"
#include "source_file/token.h"

using namespace tanlang;

SourceManager::SourceManager(str filename, vector<Token *> tokens)
    : _filename(std::move(filename)), _tokens(std::move(tokens)) {
  if (_tokens.empty()) { // if empty, insert a token so that source location 0:0 is always valid
    auto *f = new SourceFile();
    f->from_string("\n");
    _tokens.push_back(new Token(TokenType::COMMENTS, 0, 0, "", f));
  }
}

Token *SourceManager::get_token(uint32_t loc) const {
  TAN_ASSERT(loc < _tokens.size());
  return _tokens[loc];
}

uint32_t SourceManager::get_line(uint32_t loc) const { return get_token(loc)->get_line() + 1; }

uint32_t SourceManager::get_col(uint32_t loc) const { return get_token(loc)->get_col() + 1; }

str SourceManager::get_token_str(uint32_t loc) const { return get_token(loc)->get_value(); }

Token *SourceManager::get_last_token() const { return _tokens.back(); }

bool SourceManager::is_eof(uint32_t loc) const { return loc >= _tokens.size(); }

str SourceManager::get_source_code(uint32_t start, uint32_t end) const {
  str ret;

  Token *start_tok = get_token(start);
  Token *end_tok = get_token(end);

  TAN_ASSERT(start_tok->src() == end_tok->src());

  return start_tok->src()->substr(Token::GetSrcLoc(start_tok), ++Token::GetSrcLoc(end_tok));
}

str SourceManager::get_filename() const { return _filename; }

str SourceManager::get_src_location_str(uint32_t loc) const {
  return get_filename() + ":" + std::to_string(get_line(loc));
}

SourceFile *SourceManager::src() const { return _tokens[0]->src(); }
