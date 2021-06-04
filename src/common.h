#ifndef TAN_SRC_AST_COMMON_H_
#define TAN_SRC_AST_COMMON_H_
#include "base.h"
#include "src/llvm_include.h"
#include "src/ast/ast_node_type.h"

namespace tanlang {

/**
 * \brief create_ty an `alloca` instruction in the beginning of a block.
 * \param block BasicBlock to insert to.
 * \param type Intended type to store.
 * \param name Name of the `alloca` instruction.
 * \param size size of the array if greater than 1
 */
AllocaInst *create_block_alloca(BasicBlock *block, Type *type, size_t size = 1, const str &name = "");

bool is_ast_type_in(ASTNodeType t, const vector<ASTNodeType> &list);

bool is_string_in(const str &s, const vector<str> &list);

template<size_t N> bool is_ast_type_in(ASTNodeType t, std::array<ASTNodeType, N> list) {
  return std::any_of(list.begin(), list.end(), [t](ASTNodeType i) { return i == t; });
}

} // namespace tanlang

#endif /* TAN_SRC_AST_COMMON_H_ */
