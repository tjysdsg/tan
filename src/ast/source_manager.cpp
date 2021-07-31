#include "source_manager.h"
#include "src/ast/ast_base.h"
#include "token.h"

using namespace tanlang;

SourceManager::SourceManager(str filename, vector<Token *> tokens)
    : _filename(std::move(filename)), _tokens(std::move(tokens)) {
  if (_tokens.empty()) { /// if the file is empty, insert a token so that source location 0:0 is always valid
    _tokens.push_back(new Token(TokenType::COMMENTS, 0, 0, "", ""));
  }
}

Token *SourceManager::get_token(SourceIndex loc) const {
  if (loc._index >= _tokens.size()) {
    Error err("Invalid source location {filename}:{line}");
    err.raise();
  }
  return _tokens[loc._index];
}

size_t SourceManager::get_line(SourceIndex loc) const {
  return get_token(loc)->get_line() + 1;
}

size_t SourceManager::get_col(SourceIndex loc) const {
  return get_token(loc)->get_col() + 1;
}

str SourceManager::get_token_str(SourceIndex loc) const {
  return get_token(loc)->get_value();
}

Token *SourceManager::get_last_token() const { return _tokens.back(); }

bool SourceManager::is_eof(SourceIndex loc) const { return loc._index >= _tokens.size(); }

str SourceManager::get_source_code(ASTBase *p) const {
  // TODO: return the string that also covers the child nodes, instead of only p's own token str
  return get_token_str(p->get_loc());
}
