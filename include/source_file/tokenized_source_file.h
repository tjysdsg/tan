#ifndef __TAN_SRC_AST_SOURCE_MANAGER_H__
#define __TAN_SRC_AST_SOURCE_MANAGER_H__
#include "base.h"

namespace tanlang {

/**
 * \brief Different from SourceFile, TokenizedSourceFile manages the tokenized text of a source file.
 */
class TokenizedSourceFile {
public:
  TokenizedSourceFile() = delete;
  TokenizedSourceFile(str filename, vector<Token *> tokens);
  Token *get_token(uint32_t loc) const;
  uint32_t get_line(uint32_t loc) const;
  uint32_t get_col(uint32_t loc) const;
  str get_token_str(uint32_t loc) const;
  Token *get_last_token() const;
  bool is_eof(uint32_t loc) const;
  str get_filename() const;
  str get_src_location_str(uint32_t loc) const;
  SourceFile *src() const;

  str get_source_code(uint32_t start, uint32_t end) const;

private:
  str _filename;
  vector<Token *> _tokens;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_SOURCE_MANAGER_H__
