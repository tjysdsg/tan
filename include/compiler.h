#ifndef TAN_INCLUDE_COMPILER_H_
#define TAN_INCLUDE_COMPILER_H_
#include "src/llvm_include.h"

struct TanCompilation;

namespace tanlang {

class ASTNode;
class CompilerSession;

class Compiler {
public:
  static void ParseFile(const std::string &filename);
  static TargetMachine *GetDefaultTargetMachine();
  static inline std::vector<std::string> import_dirs{};
  static std::vector<std::string> resolve_import(const std::string &callee_path, const std::string &import_name);

private:
  static inline std::unordered_map<std::string, CompilerSession *> sessions{};
  /// created by import statement, used only for parsing
  static inline std::vector<std::shared_ptr<Compiler>> sub_compilers{};
  static inline TargetMachine *target_machine = nullptr;

public:
  Compiler() = delete;
  explicit Compiler(const std::string &filename);
  ~Compiler();
  void parse();
  Value *codegen();
  void emit_object(const std::string &filename);
  void dump_ir() const;
  void dump_ast() const;

private:
  CompilerSession *_compiler_session = nullptr; /// wrapper for various LLVM classes
  std::shared_ptr<ASTNode> _ast = nullptr;
  std::string _filename = "";
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
