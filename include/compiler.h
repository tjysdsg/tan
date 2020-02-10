#ifndef TAN_INCLUDE_COMPILER_H_
#define TAN_INCLUDE_COMPILER_H_
#include "src/compiler/interpreter.h"
#include "src/llvm_include.h"

namespace tanlang {

class Compiler {
 public:
  Compiler() = delete;
  explicit Compiler(Module *module);

  void emit_object(const std::string &filename);

 private:
  Module *_llvm_module;
  llvm::TargetMachine *_target_machine = nullptr;
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
