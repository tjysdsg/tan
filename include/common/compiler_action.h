#ifndef __TAN_COMMON_COMPILER_ACTION_H__
#define __TAN_COMMON_COMPILER_ACTION_H__

#include "common/ast_visitor.h"

namespace tanlang {

template <typename Derived> class CompilerAction : public ASTVisitor<Derived> {
public:
  void run(Program *p) { ((Derived *)p)->run_impl(p); }
};

} // namespace tanlang

#endif //__TAN_INCLUDE_COMMON_COMPILER_ACTION_H__
