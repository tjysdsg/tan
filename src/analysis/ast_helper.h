#ifndef __TAN_SRC_ANALYSIS_AST_HELPER_H__
#define __TAN_SRC_ANALYSIS_AST_HELPER_H__
#include "base.h"

namespace tanlang {

AST_FWD_DECL(ASTTy);
AST_FWD_DECL(ASTNode);

class ASTHelper {
public:
  ASTHelper(CompilerSession *cs);

  ASTNodePtr get_id_referred(const ASTNodePtr &p) const;
  str get_source_location(ASTNodePtr p) const;
  ASTTyPtr get_contained_ty(const ASTTyPtr &p) const;
  ASTTyPtr get_ptr_to(const ASTTyPtr &p) const;
  size_t get_struct_member_index(const ASTTyPtr &p, const str &name) const;
  ASTTyPtr get_struct_member_ty(const ASTTyPtr &p, size_t i) const;
  ASTTyPtr get_ty(const ParsableASTNodePtr &p) const;
  ASTNodePtr try_convert_to_ast_node(const ParsableASTNodePtr &p) const;

private:
  CompilerSession *_cs = nullptr;
};

}

#endif //__TAN_SRC_ANALYSIS_AST_HELPER_H__
