#ifndef TAN_INCLUDE_COMPILER_H_
#define TAN_INCLUDE_COMPILER_H_
#include "src/llvm_include.h"

struct TanCompilation;

namespace tanlang {

class ASTNode;

class CompilerSession;

class Compiler {
public:
  Compiler() = delete;
  Compiler(std::string filename, std::shared_ptr<ASTNode> ast, TanCompilation *config);
  ~Compiler();
  Value *codegen();

  void emit_object(const std::string &filename);

  void dump() const { _llvm_module->print(llvm::outs(), nullptr); }

private:
  Module *_llvm_module = nullptr;
  llvm::TargetMachine *_target_machine = nullptr;
  CompilerSession *_compiler_session = nullptr;
  std::shared_ptr<ASTNode> _ast = nullptr;
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
