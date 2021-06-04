#ifndef __TAN_SRC_ANALYSIS_AST_HELPER_H__
#define __TAN_SRC_ANALYSIS_AST_HELPER_H__
#include "base.h"
#include "src/ast/fwd.h"

namespace tanlang {

class ASTHelper {
public:
  ASTHelper(CompilerSession *cs);

  str get_source_location(SourceTraceable *p) const;
  ASTType *get_contained_ty(ASTType *p) const;
  ASTType *get_ptr_to(ASTType *p) const;

private:
  CompilerSession *_cs = nullptr;
};

}

#endif //__TAN_SRC_ANALYSIS_AST_HELPER_H__
