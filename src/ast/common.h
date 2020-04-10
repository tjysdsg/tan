#ifndef TAN_SRC_AST_COMMON_H_
#define TAN_SRC_AST_COMMON_H_
#include "parser.h"
#include "src/ast/astnode.h"
#include "src/llvm_include.h"

namespace tanlang {
/**
 * \brief Create an `alloca` instruction in the beginning of a block.
 * \param block BasicBlock to insert to.
 * \param type Intended type to store.
 * \param name Name of the `alloca` instruction.
 */
AllocaInst *create_block_alloca(BasicBlock *block, Type *type, const std::string &name = "");

bool is_ast_type_in(ASTType t, std::initializer_list<ASTType> list);

}

#endif /* TAN_SRC_AST_COMMON_H_ */
