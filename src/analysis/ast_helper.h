#ifndef __TAN_SRC_ANALYSIS_AST_HELPER_H__
#define __TAN_SRC_ANALYSIS_AST_HELPER_H__
#include "base.h"

namespace tanlang {

AST_FWD_DECL(ASTType);
AST_FWD_DECL(ASTNode);
AST_FWD_DECL(SourceTraceable);

class ASTHelper {
public:
  ASTHelper(CompilerSession *cs);

  ASTNodePtr get_id_referred(const ASTNodePtr &p) const;
  str get_source_location(SourceTraceablePtr p) const;
  ASTTypePtr get_contained_ty(const ASTTypePtr &p) const;
  ASTTypePtr get_ptr_to(const ASTTypePtr &p) const;
  size_t get_struct_member_index(const ASTTypePtr &p, const str &name) const;
  ASTTypePtr get_struct_member_ty(const ASTTypePtr &p, size_t i) const;
  ASTTypePtr get_ty(const ASTBasePtr &p) const;
  ASTNodePtr try_convert_to_ast_node(const ASTBasePtr &p) const;

private:
  CompilerSession *_cs = nullptr;
};

}

#endif //__TAN_SRC_ANALYSIS_AST_HELPER_H__
