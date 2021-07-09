#ifndef __TAN_SRC_AST_SOURCE_TRACEABLE_H__
#define __TAN_SRC_AST_SOURCE_TRACEABLE_H__
#include "base.h"
#include "src/ast/source_manager.h"

namespace tanlang {

/**
 * \brief Represents the nodes that can be traced back to the token at the exact location in the source file.
 */
class SourceTraceable {
public:
  SourceTraceable() = delete;
  SourceTraceable(SourceIndex source_loc);
  const SourceIndex &get_loc() const;
  void set_loc(SourceIndex loc);

protected:
  SourceIndex _loc = nullptr;
};

}

#endif //__TAN_SRC_AST_SOURCE_TRACEABLE_H__
