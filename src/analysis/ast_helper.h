#ifndef __TAN_SRC_ANALYSIS_AST_HELPER_H__
#define __TAN_SRC_ANALYSIS_AST_HELPER_H__
#include "base.h"

namespace tanlang {

AST_FWD_DECL(ASTType);
AST_FWD_DECL(SourceTraceable);

class ASTHelper {
public:
  ASTHelper(CompilerSession *cs);

  str get_source_location(SourceTraceablePtr p) const;
  ASTTypePtr get_contained_ty(const ASTTypePtr &p) const;
  ASTTypePtr get_ptr_to(const ASTTypePtr &p) const;

private:
  CompilerSession *_cs = nullptr;
};

}

#endif //__TAN_SRC_ANALYSIS_AST_HELPER_H__
