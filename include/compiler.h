#ifndef TAN_INCLUDE_COMPILER_H_
#define TAN_INCLUDE_COMPILER_H_
#include "src/llvm_include.h"

struct TanCompilation;

namespace tanlang {

class ASTNode;

class CompilerSession;

class Compiler {
public:
  static CompilerSession *get_compiler_session(const std::string &filename);
  static void set_compiler_session(const std::string &filename, CompilerSession *compiler_session);
  static void ParseFile(const std::string filename);

private:
  static std::unordered_map<std::string, CompilerSession *> sessions;
  /// created by import statement, used only for parsing
  static std::vector<std::shared_ptr<Compiler>> sub_compilers;

public:
  Compiler() = delete;
  Compiler(std::string filename);
  ~Compiler();

  void parse();
  Value *codegen();
  void emit_object(const std::string &filename);
  void dump_ir() const;
  void dump_ast() const;

private:
  llvm::TargetMachine *_target_machine = nullptr;
  CompilerSession *_compiler_session = nullptr;
  std::shared_ptr<ASTNode> _ast = nullptr;
  std::string _filename = "";
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
