#ifndef __TAN_SRC_AST_AST_STRUCT_H__
#define __TAN_SRC_AST_AST_STRUCT_H__
#include "base.h"
#include "src/ast/ast_type.h"

namespace tanlang {

class ASTStruct : public ASTType {
private:
  umap<str, size_t> _member_indices{};
  vector<str> _member_names{};
};

}

#endif //__TAN_SRC_AST_AST_STRUCT_H__
