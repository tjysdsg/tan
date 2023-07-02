#ifndef TAN_SRC_LIB_LIBTANC_H_
#define TAN_SRC_LIB_LIBTANC_H_
#include "base.h"

/**
 * \file libtanc.h
 * \brief Defines tools for compiling tan source files.
 * */

namespace tanlang {

class Parser;
class Compiler;

/**
 * \enum TanCompileType
 * \brief Output file type
 */
enum TanCompileType {
  OBJ = 0, /// Object file, use as default
  EXE,     /// Executable
  SLIB,    /// Static library
  DLIB,    /// Shared library
};

/**
 * \enum TanOptLevel
 * \brief Optimization level
 * \details The values must match llvm::CodeGenOpt::Level
 */
enum TanOptLevel {
  O0 = 0, /// Debug
  O1 = 1, /// Less
  O2 = 2, /// Default
  O3 = 3, /// Aggressive
};

/**
 * \struct TanCompilation
 * \brief Compilation configuration
 */
struct TanCompilation {
  TanCompileType type = OBJ;  /// Type of compilation, \see TanCompileType
  TanOptLevel opt_level = O0; /// Optimization level, \see TanOptLevel
  unsigned verbose = 0;     /// Verbose level, 0 non-verbose, 1 print LLVM IR, 2, print LLVM IR and abstract syntax tree
  str out_file = "a.out";   /// Output filename, invalid if TanCompilation::type is set to OBJ
  vector<str> link_files{}; /// Files to link against
  vector<str> lib_dirs{};   /// Library search paths
  vector<str> import_dirs{}; /// Search import paths
  // TODO: output platform, architecture, ...
};

/**
 * \brief Initialize compiler
 * \note This is required before calling compile_files
 */
bool init_compiler(int argc, char **argv);

/**
 * \brief Compile multiple source files.
 * \details The output files are named as "<name of the source file>.o" and they are located at current working
 * directory. If current build is release, all exceptions are captured and `e.what()` is printed out to stderr. If
 * current build is debug, all exceptions are not captured, making debugging easier. \param n_files The number of source
 * files. \param input_paths The path of the input files, can be relative or absolute path. The input files can be tan
 * source files, or object files. They will be distinguished by their file extensions,
 *  ".tan" and ".o".
 * \param config Compilation configuration, \see TanCompilation
 * \return If current build is release, returns true if no error occurred, and vice versa.
 *  If current build is debug, either returns true or doesn't return, because all errors are captured by
 *         a try-catch clause.
 */
bool compile_files(vector<str> input_paths, TanCompilation *config);

inline str opt_level_to_string(TanOptLevel l) {
  switch (l) {
  case O0:
    return "-O0";
  case O1:
    return "-O1";
  case O2:
    return "-O2";
  case O3:
    return "-O3";
  default:
    TAN_ASSERT(false);
  }
}

} // namespace tanlang

#endif /* TAN_SRC_LIB_LIBTANC_H_ */
