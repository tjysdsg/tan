#ifndef TAN_SRC_LLVM_INCLUDE_H_
#define TAN_SRC_LLVM_INCLUDE_H_
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/APInt.h>
#include "llvm/ADT/StringRef.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DebugInfo.h>

namespace tanlang {

using llvm::Expected;
using llvm::Error;
using llvm::StringRef;
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
using llvm::ArrayType;
using llvm::IRBuilder;
using llvm::BasicBlock;
using llvm::Module;
using llvm::DataLayout;
using llvm::LLVMContext;
using llvm::GlobalVariable;
using llvm::GlobalValue;
using llvm::ConstantPointerNull;
using llvm::ConstantStruct;
using llvm::ConstantArray;
using llvm::StructType;
using llvm::PointerType;

using llvm::SectionMemoryManager;
using llvm::legacy::FunctionPassManager;

using llvm::DIBuilder;
using llvm::DICompileUnit;
using llvm::DIType;

}

#endif /* TAN_SRC_LLVM_INCLUDE_H_ */
