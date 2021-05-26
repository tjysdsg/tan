#ifndef TAN_SRC_AST_COMMON_H_
#define TAN_SRC_AST_COMMON_H_
#include "base.h"
#include "src/llvm_include.h"
#include "src/ast/ast_type.h"

namespace tanlang {

/**
 * \brief create_ty an `alloca` instruction in the beginning of a block.
 * \param block BasicBlock to insert to.
 * \param type Intended type to store.
 * \param name Name of the `alloca` instruction.
 * \param size size of the array if greater than 1
 */
AllocaInst *create_block_alloca(BasicBlock *block, Type *type, size_t size = 1, const str &name = "");

bool is_ast_type_in(ASTType t, std::initializer_list<ASTType> list);

bool is_string_in(std::string_view, std::initializer_list<std::string_view>);

bool is_string_in(std::string_view s, const vector<str> &list);

template<size_t N> bool is_ast_type_in(ASTType t, std::array<ASTType, N> list) {
  return std::any_of(list.begin(), list.end(), [t](ASTType i) { return i == t; });
}

} // namespace tanlang

#endif /* TAN_SRC_AST_COMMON_H_ */
