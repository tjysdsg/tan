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

/**
 * \brief Parse, Analyze, and compile a list of tan source files. The compilation consists of multiple stages,
 *        performed using the CompilerAction interface.
 */
class Compiler {
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
   *    the call to resolve_import should be like `Compiler::resolve_import("./src.tan", "../parent.tan")`
   * \param callee_path The path to the file which the import statement is in
   * \param import_name The filename specified by the import statement
   */
  static vector<str> resolve_import(const str &callee_path, const str &import_name);

private:
  /**
   * \brief Compiler instances created due to import statements
   * \details These instances do NOT generate any code, they only serve as a parser
   */
  static inline llvm::TargetMachine *target_machine = nullptr;

public:
  Compiler() = delete;

  explicit Compiler(const str &filename);
  ~Compiler();

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
  llvm::Value *codegen();

  /**
   * \brief Compile to object files
   * \details Resulting *.o files are stored in the current working directory
   */
  void emit_object(const str &filename);

  /**
   * \brief Print LLVM IR code
   */
  void dump_ir() const;

  /**
   * \brief Pretty-print AST
   */
  void dump_ast() const;

  Program *get_root_ast() const;

private:
  Program *_ast = nullptr;
  str _filename;
  SourceManager *_sm = nullptr;
  CodeGenerator *_cg = nullptr;
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
