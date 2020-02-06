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
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Pass.h>
#include <llvm/IR/LegacyPassManager.h>
#include "llvm/IR/DataLayout.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"

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
using llvm::IRBuilder;
using llvm::BasicBlock;
using llvm::Module;
using llvm::DataLayout;
using llvm::LLVMContext;

using llvm::orc::ThreadSafeModule;
using llvm::orc::ExecutionSession;
using llvm::orc::RTDyldObjectLinkingLayer;
using llvm::orc::IRCompileLayer;
using llvm::orc::MangleAndInterner;
using llvm::orc::ThreadSafeContext;
using llvm::orc::JITTargetMachineBuilder;
using llvm::JITEvaluatedSymbol;
using llvm::orc::ConcurrentIRCompiler;
using llvm::orc::DynamicLibrarySearchGenerator;
using llvm::orc::JITDylib;

using llvm::SectionMemoryManager;
}

#endif /* TAN_SRC_LLVM_INCLUDE_H_ */
