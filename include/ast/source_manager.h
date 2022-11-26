#ifndef __TAN_SRC_AST_SOURCE_MANAGER_H__
#define __TAN_SRC_AST_SOURCE_MANAGER_H__
#include "base.h"

namespace tanlang {

class ASTBase;

/**
 * \brief Stores the location information of a token in SourceManager
 */
class SrcLoc {
public:
  friend class SourceManager;

  SrcLoc() = delete;
  SrcLoc(const SrcLoc &) = default;
  SrcLoc &operator=(const SrcLoc &) = default;

  explicit SrcLoc(size_t index) { _index = index; }
  static SrcLoc CreateInvalidIndex() { return SrcLoc(static_cast<size_t>(-1)); }
  size_t get_index() const { return _index; }
  void offset_by(int64_t offset) { _index = (size_t)((int64_t)_index + offset); }

private:
  size_t _index = 0;
};

class SourceManager {
public:
  SourceManager() = delete;
  SourceManager(str filename, vector<Token *> tokens);
  Token *get_token(SrcLoc loc) const;
  size_t get_line(SrcLoc loc) const;
  size_t get_col(SrcLoc loc) const;
  str get_token_str(SrcLoc loc) const;
  Token *get_last_token() const;
  bool is_eof(SrcLoc loc) const;
  str get_source_code(ASTBase *p) const;

private:
  str _filename;
  vector<Token *> _tokens;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_SOURCE_MANAGER_H__
