#ifndef TAN_SRC_LLVM_INCLUDE_H_
#define TAN_SRC_LLVM_INCLUDE_H_
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instruction.h>

namespace tanlang {

using llvm::Value;
using llvm::AllocaInst;
using llvm::Type;
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::APFloat;
using llvm::APInt;
using llvm::Function;
using llvm::PHINode;
using llvm::verifyFunction;
using llvm::FunctionType;
using llvm::IRBuilder;
using llvm::BasicBlock;

}

#endif /* TAN_SRC_LLVM_INCLUDE_H_ */
