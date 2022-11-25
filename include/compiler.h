#ifndef TAN_INCLUDE_COMPILER_H_
#define TAN_INCLUDE_COMPILER_H_
#include "base.h"
#include "libtanc.h"
#include "ast/fwd.h"

namespace llvm {
class TargetMachine;
class Value;
}

namespace tanlang {

class CompilerSession;

/**
 * \class Compiler
 * \brief Abstraction of a compiler
 * */
class Compiler {
public:
  /**
   * \brief Parse a single file to AST
   * */
  static void ParseFile(const str &filename);

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
   * \brief CompilerSession object of each source file
   * */
  static inline umap<str, CompilerSession *> sessions{};

  /**
   * \brief Compiler instances created due to import statements
   * \details These instances do NOT generate any code, they only serve as a parser
   * */
  static inline vector<Compiler *> sub_compilers{};
  static inline llvm::TargetMachine *target_machine = nullptr;

public:
  Compiler() = delete;
  /**
   * \brief create_ty a Compiler instance with its relevant source file name/path
   * */
  explicit Compiler(const str &filename);
  ~Compiler();

  /**
   * \brief Parse the corresponding source file, and build AST
   * */
  void parse();

  /**
   * \brief Generate LLVM IR code
   * */
  llvm::Value *codegen();

  /**
   * \brief Emit object files
   * \details A file called "<filename>.o" will be created in the current working directory
   * */
  void emit_object(const str &filename);

  /**
   * \brief Print LLVM IR code
   * */
  void dump_ir() const;

  /**
   * \brief Pretty-print AST
   * */
  void dump_ast() const;

private:
  CompilerSession *_cs = nullptr;
  ASTContext *_ctx = nullptr;
  ASTBase *_ast = nullptr;
  str _filename = "";
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
