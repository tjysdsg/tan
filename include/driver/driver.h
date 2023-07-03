#ifndef TAN_INCLUDE_COMPILER_H_
#define TAN_INCLUDE_COMPILER_H_
#include "base.h"
#include "tan/tan.h"

namespace llvm {
class TargetMachine;
class Value;
} // namespace llvm

namespace tanlang {

class CodeGenerator;
class Program;
class SourceManager;
class CompilationUnit;
class SourceFile;

/**
 * \brief Parse, Analyze, and compile a list of C++ or tan source files.
 *       The compilation consists of multiple stages, performed using the CompilerAction interface.
 */
class CompilerDriver final {
public:
  /**
   * \brief Get information about the current machine
   * */
  static llvm::TargetMachine *GetDefaultTargetMachine();

  /**
   * \brief Import search directories
   * \details This is set by compile_files() in libtanc.h
   */
  static inline vector<str> import_dirs{};

  /**
   * \brief Current compile configuration
   */
  static inline TanCompilation compile_config{};

  /**
   * \brief Get a list of possible files that corresponds to an import
   * \details Suppose there's an import statement `import "../parent.tan"`, in a file at "./src.tan",
   *    the call to resolve_import should be like `CompilerDriver::resolve_import("./src.tan", "../parent.tan")`
   * \param callee_path The path to the file which the import statement is in
   * \param import_name The filename specified by the import statement
   */
  static vector<str> resolve_import(const str &callee_path, const str &import_name);

private:
  /**
   * \brief CompilerDriver instances created due to import statements
   * \details These instances do NOT generate any code, they only serve as a parser
   */
  static inline llvm::TargetMachine *target_machine = nullptr;

public:
  CompilerDriver() = delete;

  explicit CompilerDriver(const vector<str> &files);
  ~CompilerDriver();

  /**
   * \brief Parse the corresponding source file, and build AST
   */
  void parse();

  /**
   * \brief Perform Semantic Analysis
   */
  void analyze();

  /**
   * \brief Generate LLVM IR
   */
  llvm::Value *codegen(CompilationUnit *cu, bool print_ir);

  /**
   * \brief Compile to object files
   * \details Resulting *.o files are stored in the current working directory
   */
  void emit_object(CompilationUnit *cu, const str &out_file);

  /**
   * \brief Pretty-print AST
   */
  void dump_ast() const;

  const vector<CompilationUnit *> &get_compilation_units() const;

private:
  vector<str> _files{};
  vector<SourceFile *> _srcs{};
  vector<CompilationUnit *> _cu{};
  umap<CompilationUnit *, CodeGenerator *> _cg{};
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
