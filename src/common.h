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

#define switch_str(s) auto& this_variable_name_is_intended_to_be_long_to_avoid_name_conflicts = s;
#define case_str0(s) if (this_variable_name_is_intended_to_be_long_to_avoid_name_conflicts == (s)) {
#define case_str(s) } else if (this_variable_name_is_intended_to_be_long_to_avoid_name_conflicts == (s)) {
#define case_str_or2(s1, s2) } else if (this_variable_name_is_intended_to_be_long_to_avoid_name_conflicts == (s1) \
                                  || this_variable_name_is_intended_to_be_long_to_avoid_name_conflicts == (s2)) {
#define case_str_or3(s1, s2, s3) } else if (this_variable_name_is_intended_to_be_long_to_avoid_name_conflicts == (s1) \
                                      || this_variable_name_is_intended_to_be_long_to_avoid_name_conflicts == (s2) \
                                      || this_variable_name_is_intended_to_be_long_to_avoid_name_conflicts == (s3)) {
#define case_default } else {
#define end_switch }
} // namespace tanlang

#endif /* TAN_SRC_AST_COMMON_H_ */
