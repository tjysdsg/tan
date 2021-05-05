#ifndef __TAN_SRC_AST_SOURCE_TRACEABLE_H__
#define __TAN_SRC_AST_SOURCE_TRACEABLE_H__
#include "base.h"

namespace tanlang {

/**
 * \brief Represents the nodes that can be traced back to the token at the exact location in the source file.
 */
class SourceTraceable {

public:
  /**
   * \brief Get the line number of this node, starting from 1
   */
  size_t get_line();

  /**
   * \brief Get the column number of this node, starting from 1
   */
  size_t get_col();

  /**
   * \brief Get the token of this node at idx
   */
  Token *get_token();

  /**
   * \brief Set token
   */
  void set_token(Token *token);

  /**
   * \brief Get the string value of the token
   */
  str get_token_str();

public:
  size_t _start_index = 0;
  size_t _end_index = 0;

private:
  Token *_token = nullptr;
};

}

#endif //__TAN_SRC_AST_SOURCE_TRACEABLE_H__
