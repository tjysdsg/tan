#ifndef __TAN_SRC_AST_AST_STRUCT_H__
#define __TAN_SRC_AST_AST_STRUCT_H__
#include "base.h"
#include "src/ast/ast_type.h"

namespace tanlang {

AST_FWD_DECL(Expr);

class ASTStruct : public ASTType {
public:
  umap<str, size_t> _member_indices{};
  vector<str> _member_names{};
  vector<ExprPtr> _initial_values{};
};

}

#endif //__TAN_SRC_AST_AST_STRUCT_H__
