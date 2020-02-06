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
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Pass.h>
#include <llvm/IR/LegacyPassManager.h>

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
using llvm::Module;

}

#endif /* TAN_SRC_LLVM_INCLUDE_H_ */
