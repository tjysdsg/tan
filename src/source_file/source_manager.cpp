#include "source_file/source_manager.h"
#include "source_file/token.h"

using namespace tanlang;

SourceManager::SourceManager(str filename, vector<Token *> tokens)
    : _filename(std::move(filename)), _tokens(std::move(tokens)) {
  if (_tokens.empty()) { /// if the file is empty, insert a token so that source location 0:0 is always valid
    _tokens.push_back(new Token(TokenType::COMMENTS, 0, 0, "", new SourceFile()));
  }
}

Token *SourceManager::get_token(uint32_t loc) const {
  if (loc >= _tokens.size()) {
    Error err("Invalid source location {filename}:{line}");
    err.raise();
  }
  return _tokens[loc];
}

uint32_t SourceManager::get_line(uint32_t loc) const { return get_token(loc)->get_line() + 1; }

uint32_t SourceManager::get_col(uint32_t loc) const { return get_token(loc)->get_col() + 1; }

str SourceManager::get_token_str(uint32_t loc) const { return get_token(loc)->get_value(); }

Token *SourceManager::get_last_token() const { return _tokens.back(); }

bool SourceManager::is_eof(uint32_t loc) const { return loc >= _tokens.size(); }

str SourceManager::get_source_code(uint32_t loc) const {
  // TODO: return the string that also covers the child nodes, instead of only p's own token str
  return get_token_str(loc);
}

str SourceManager::get_filename() const { return _filename; }

str SourceManager::get_src_location_str(uint32_t loc) const {
  return get_filename() + ":" + std::to_string(get_line(loc));
}

SourceFile *SourceManager::src() const { return _tokens[0]->src(); }