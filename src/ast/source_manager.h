#ifndef __TAN_SRC_AST_SOURCE_MANAGER_H__
#define __TAN_SRC_AST_SOURCE_MANAGER_H__
#include "base.h"

namespace tanlang {

/**
 * \brief The index of the token in SourceManager
 */
class SourceIndex {
public:
  friend class SourceManager;

  SourceIndex() = delete;
  SourceIndex(size_t index) { _index = index; }
  static SourceIndex CreateInvalidIndex() { return SourceIndex(static_cast<size_t>(-1)); }

private:
  size_t _index = 0;
};

class SourceManager {
public:
  SourceManager() = delete;
  SourceManager(str filename, vector<Token *> tokens);
  Token *get_token(SourceIndex loc) const;
  size_t get_line(SourceIndex loc) const;
  size_t get_col(SourceIndex loc) const;
  str get_token_str(SourceIndex loc) const;

private:
  str _filename;
  vector<Token *> _tokens;
};

}

#endif //__TAN_SRC_AST_SOURCE_MANAGER_H__
