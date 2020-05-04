#ifndef TAN_SRC_AST_COMMON_H_
#define TAN_SRC_AST_COMMON_H_
#include "parser.h"
#include "src/ast/ast_node.h"
#include "src/llvm_include.h"

namespace tanlang {

/**
 * \brief Create an `alloca` instruction in the beginning of a block.
 * \param block BasicBlock to insert to.
 * \param type Intended type to store.
 * \param name Name of the `alloca` instruction.
 * \param size size of the array if greater than 1
 */
AllocaInst *create_block_alloca(BasicBlock *block, Type *type, size_t size = 1, const std::string &name = "");

bool is_ast_type_in(ASTType t, std::initializer_list<ASTType> list);

bool is_string_in(std::string_view, std::initializer_list<std::string_view>);

template<size_t N> bool is_ast_type_in(ASTType t, std::array<ASTType, N> list) {
  return std::any_of(list.begin(), list.end(), [t](ASTType i) { return i == t; });
}

bool is_llvm_type_same(llvm::Type *t1, llvm::Type *t2);

#define switch_str(s) auto& jldskajfdsjkahjhkjxhkjhckhvcx = s;
#define case_str0(s) if (jldskajfdsjkahjhkjxhkjhckhvcx == (s)) {
#define case_str(s) } else if (jldskajfdsjkahjhkjxhkjhckhvcx == (s)) {
#define case_str_or2(s1, s2) } else if (jldskajfdsjkahjhkjxhkjhckhvcx == (s1) || jldskajfdsjkahjhkjxhkjhckhvcx == (s2)) {
#define case_str_or3(s1, s2, s3) } else if (jldskajfdsjkahjhkjxhkjhckhvcx == (s1) \
                                            || jldskajfdsjkahjhkjxhkjhckhvcx == (s2) \
                                            || jldskajfdsjkahjhkjxhkjhckhvcx == (s3)) {
#define case_default } else {
#define end_switch }
} // namespace tanlang

#endif /* TAN_SRC_AST_COMMON_H_ */
