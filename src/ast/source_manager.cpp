#include "source_manager.h"
#include "token.h"

using namespace tanlang;

SourceManager::SourceManager(str filename, vector<Token *> tokens) {
  _filename = filename;
  _tokens = tokens;
}

Token *SourceManager::get_token(SourceIndex loc) const {
  if (loc._index >= _tokens.size()) {
    report_error("Invalid source location {filename}:{line}");
  }
  return _tokens[loc._index];
}

size_t SourceManager::get_line(SourceIndex loc) const {
  return get_token(loc)->get_line();
}

size_t SourceManager::get_col(SourceIndex loc) const {
  return get_token(loc)->get_col();
}

str SourceManager::get_token_str(SourceIndex loc) const {
  return get_token(loc)->get_value();
}

Token *SourceManager::get_last_token() const { return _tokens.back(); }

bool SourceManager::is_eof(SourceIndex loc) const { return loc._index >= _tokens.size(); }
