#ifndef TAN_SRC_LIB_LIBTANC_H_
#define TAN_SRC_LIB_LIBTANC_H_
#include <cstddef>

/**
 * \file libtanc.h
 * \brief Defines tools for compiling tan source files.
 * */

namespace tanlang {
class Parser;

class Reader;

class Compiler;
}

using tanlang::Parser;
using tanlang::Reader;
using tanlang::Compiler;

extern "C" {

enum TanCompileType {
  OBJ = 0, EXE, SLIB, DLIB,
};

/**
 * \struct TanCompilation
 * \brief Compilation configuration
 * */
struct TanCompilation {
  TanCompileType type = OBJ; ///< Type of compilation, \see TanCompileType
  unsigned verbose = 0; ///< Verbose level, 0 non-verbose, 1 print LLVM IR, 2, print LLVM IR and abstract syntax tree
  const char *out_file = "a.out"; ///< output filename, invalid if TanCompilation::type is set to OBJ
  size_t n_link_files = 0;
  const char *const *link_files = nullptr;
  // TODO: output platform, architecture, ...
};

/**
 * \brief Compile multiple source files.
 * \details The output files are named as "<name of the source file>.o" and they are located at current working directory.
 *          If current build is release, all exceptions are captured and `e.what()` is printed out to stderr.
 *          If current build is debug, all exceptions are not captured, making debugging easier.
 * \param n_files the number of source files.
 * \param input_paths the path of the source files, can be relative or absolute path.
 * \param config Compilation configuration, \see TanCompilation
 * \return If current build is release, returns true if no error occurred, and vice versa.
 *  If current build is debug, either returns true or doesn't return, because all errors are captured by
 *         a try-catch clause.
 * */
bool compile_files(unsigned n_files, char **input_paths, TanCompilation *config);

Parser *parse_file(const char *path);

}

#endif /* TAN_SRC_LIB_LIBTANC_H_ */
