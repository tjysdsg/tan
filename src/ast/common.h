#ifndef TAN_SRC_AST_COMMON_H_
#define TAN_SRC_AST_COMMON_H_
#include "parser.h"
#include "src/ast/astnode.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instruction.h>

namespace tanlang {
using llvm::AllocaInst;
using llvm::Function;

/**
 * \brief Create an alloca instruction in the entry block of
 * the function. This is used for mutable variables etc.
 */
AllocaInst *CreateEntryBlockAlloca(Function *func, const std::string &name, ParserContext *parser_context);

bool is_ast_type_in(ASTType t, std::initializer_list<ASTType> list);

}

#endif /* TAN_SRC_AST_COMMON_H_ */
