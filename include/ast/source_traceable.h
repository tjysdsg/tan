#ifndef __TAN_SRC_AST_SOURCE_TRACEABLE_H__
#define __TAN_SRC_AST_SOURCE_TRACEABLE_H__
#include "base.h"
#include "source_manager.h"

namespace tanlang {

/**
 * \brief Represents the nodes that can be traced back to the token at the exact location in the source file.
 */
class SourceTraceable {
public:
  SourceTraceable() = delete;
  explicit SourceTraceable(SrcLoc source_loc);
  [[nodiscard]] const SrcLoc &loc() const;
  void set_loc(SrcLoc loc);

protected:
  SrcLoc _loc = SrcLoc(0);
};

} // namespace tanlang

#endif //__TAN_SRC_AST_SOURCE_TRACEABLE_H__
