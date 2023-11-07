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
class TokenizedSourceFile;
class SourceFile;
class Package;

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
   * FIXME: static variable?
   */
  static inline vector<str> import_dirs{};

  /**
   * \brief Get a list of possible files that corresponds to an import. Check PACKAGES.md
   * \param callee_path The path to the file which the import statement is in
   * \param import_name The filename specified by the import statement
   * \return A list of absolute paths to candidate files
   */
  static vector<str> resolve_package_import(const str &callee_path, const str &import_name);

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
  vector<Program *> parse(const vector<str> &files);

  void link(const vector<str> &input_paths);

  /**
   * \brief Register a Package that has been spotted from source files, with top-level context stored inside.
   */
  void register_package(const str &name, Package *package);

  /**
   * \brief Get a pointer to a Package. Semantic analysis is not guaranteed to be fully performed on it.
   */
  Package *get_package(const str &name);

  /**
   * \brief Get a set of partially analyzed packages that can be used for cross-package dependency analysis.
   *        Note that some composite types need a full analysis for the dependent packages to work during codegen.
   * \details Package dependencies are analyzed recursively but there's no guarantee that all of them are found.
   * \note This function raises an error if it detects cyclic dependency.
   * \param ps A list of Program's returned by the Parser
   * \return A list of partially analyzed packages
   */
  vector<Package *> stage1_analysis(vector<Program *> ps);

private:
  /**
   * \brief Compile TAN files and return a list of object files
   */
  vector<str> compile_tan(const vector<str> &files);

private:
  TanCompilation _config{};
  llvm::TargetMachine *_target_machine = nullptr;

  umap<str, Package *> _packages{}; // including external dependencies, which are likely only partially analyzed

  enum class AnalyzeStatus : int {
    None = 0, // umap default value relies on this
    Processing,
    Done,
  };
  umap<str, AnalyzeStatus> _package_status{}; // used to avoid recursive importing
};

} // namespace tanlang

#endif /* TAN_INCLUDE_COMPILER_H_ */
