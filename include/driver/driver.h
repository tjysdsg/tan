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
 * \brief Compile a list of C++ and/or tan source files, and perform linking.
 */
class CompilerDriver final {
public:
  static CompilerDriver *instance() { return singleton; }

private:
  static inline CompilerDriver *singleton = nullptr;

public:
  /**
   * \brief Import search directories
   * \details This is set by compile_files() in libtanc.h
   */
  static inline vector<str> import_dirs{};

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
  explicit CompilerDriver(TanCompilation config);
  ~CompilerDriver();

  /**
   * \brief Compile CXX or TAN source files and link their output object files
   * \details The object files are named as "<name of the source file>.o" and they are located at current working
   *          directory.
   *          If current build is release, all exceptions are captured and `e.what()` is printed out to stderr.
   *          If current build is debug, all exceptions are not captured, making debugging easier.
   *
   * \param files The path to source files, can be relative or absolute path.
   *              They will be distinguished by their file extensions.
   * \param config Compilation configuration, \see TanCompilation
   */
  void run(const vector<str> &files);

  /**
   * \brief Parse the corresponding source file, and build AST
   */
  vector<CompilationUnit *> parse(const vector<str> &files);

  /**
   * \brief Perform Semantic Analysis
   */
  void analyze(vector<CompilationUnit *> cu);

  /**
   * \brief Generate LLVM IR
   */
  CodeGenerator *codegen(CompilationUnit *cu, bool print_ir);

  /**
   * \brief Compile to object files
   * \details Resulting *.o files are stored in the current working directory
   */
  void emit_object(CodeGenerator *cg, const str &out_file);

  void link(const vector<str> &input_paths);

private:
  /**
   * \brief Compile TAN files and return a list of object files
   */
  vector<str> compile_tan(const vector<str> &files);

private:
  TanCompilation _config{};
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
