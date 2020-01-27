#ifndef TAN_SRC_AST_COMMON_H_
#define TAN_SRC_AST_COMMON_H_
#include "parser.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instruction.h>

namespace tanlang {
using llvm::Value;
using llvm::Type;
using llvm::AllocaInst;
using llvm::Function;

/**
 * \brief Create an alloca instruction in the entry block of
 * the function. This is used for mutable variables etc.
 */
static AllocaInst *CreateEntryBlockAlloca(Function *func, const std::string &name, ParserContext *parser_context) {
  IRBuilder<> tmp_builder(&func->getEntryBlock(), func->getEntryBlock().begin());
  return tmp_builder.CreateAlloca(parser_context->_builder->getFloatTy(), nullptr, name);
}
}

#endif /* TAN_SRC_AST_COMMON_H_ */
