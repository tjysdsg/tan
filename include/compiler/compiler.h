#ifndef TAN_INCLUDE_COMPILER_H_
#define TAN_INCLUDE_COMPILER_H_
#include "base.h"
#include "tan/tan.h"
#include "parser/parsed_module.h"

namespace llvm {
class TargetMachine;
class Value;
} // namespace llvm

namespace tanlang {

class CodeGenerator;
class Program;
class SourceManager;
class Package;
class Context;

/**
 * \brief Compiler is responsible for parsing, type checking, and generating machine code of the input files.
 *        It parse all files independently, group them into packages, and generating one object file for each package.
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
   * */
  static inline vector<str> import_dirs{};

  /**
   * \brief Current compile configuration
   * */
  static inline TanCompilation compile_config{};

  /**
   * \brief Get a list of possible files that corresponds to an import
   * \details Suppose there's an import statement `import "../parent.tan"`, in a file at "./src.tan",
   *    the call to resolve_import should be like `Compiler::resolve_import("./src.tan", "../parent.tan")`
   * \param callee_path The path to the file which the import statement is in
   * \param import_name The filename specified by the import statement
   * */
  static vector<str> resolve_import(const str &callee_path, const str &import_name);

private:
  /**
   * \brief Compiler instances created due to import statements
   * \details These instances do NOT generate any code, they only serve as a parser
   * */
  static inline llvm::TargetMachine *target_machine = nullptr;

public:
  Compiler();

  /**
   * \brief Parse a source file and build AST
   * */
  void parse(const str &filename);

  /**
   * Semantic analysis, include package dependency analysis, name resolution, and type checking.
   */
  void analyze();

  /**
   * \brief Generate LLVM IR code
   * */
  llvm::Value *codegen();

  /**
   * \brief Emit object files
   * */
  void emit_objects();

  /**
   * \brief Print LLVM IR code
   * */
  void dump_ir() const;

  /**
   * \brief Pretty-print AST
   */
  void dump_ast() const;

private:
  umap<str, vector<ParsedModule>> _parsed_modules{};
  umap<str, Package *> _packages{};
  umap<str, Context *> _package_ctx{};
  CodeGenerator *_cg = nullptr;

  /**
   * \brief Merge parsed modules of the same package
   */
  void merge_parsed_modules_by_package();
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
