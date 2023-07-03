#ifndef TAN_SRC_LIB_LIBTANC_H_
#define TAN_SRC_LIB_LIBTANC_H_
#include "base.h"

/**
 * \file libtanc.h
 * \brief Defines tools for compiling tan source files.
 * */

namespace tanlang {

class Parser;
class CompilerDriver;

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
};

/**
 * \brief Initialize compiler
 * \note This is required before calling compile_files
 */
bool init_compiler(int argc, char **argv);

/**
 * \brief Get optimization level compiler flag from the enum value.
 * \note This must match clang's option string.
 */
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
