#ifndef TAN_SRC_AST_COMMON_H_
#define TAN_SRC_AST_COMMON_H_
#include "base.h"
#include "ast/ast_node_type.h"

namespace tanlang {

bool is_ast_type_in(ASTNodeType t, const vector<ASTNodeType> &list);

bool is_string_in(const str &s, const vector<str> &list);

template<size_t N> bool is_ast_type_in(ASTNodeType t, std::array<ASTNodeType, N> list) {
  return std::any_of(list.begin(), list.end(), [t](ASTNodeType i) { return i == t; });
}

} // namespace tanlang

#endif /* TAN_SRC_AST_COMMON_H_ */
