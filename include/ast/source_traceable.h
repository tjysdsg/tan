#ifndef __TAN_SRC_AST_SOURCE_TRACEABLE_H__
#define __TAN_SRC_AST_SOURCE_TRACEABLE_H__
#include "base.h"
#include "source_file/source_manager.h"

namespace tanlang {

/**
 * \brief Different from SourceSpan, TokenSpan operates on the token level.
 */
class TokenSpan {
public:
  TokenSpan() = delete;
  TokenSpan(uint32_t start, uint32_t end);

  uint32_t _start = 0;
  uint32_t _end = 0;
};

/**
 * \brief Represents the nodes that can be traced back to tokens in the source file.
 */
class SourceTraceable {
public:
  SourceTraceable() = delete;
  SourceTraceable(SourceFile *src);
  [[nodiscard]] const TokenSpan &span() const;
  void set_span(TokenSpan span);

private:
  TokenSpan _span;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_SOURCE_TRACEABLE_H__
