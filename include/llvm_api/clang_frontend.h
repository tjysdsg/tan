#ifndef __TAN_INCLUDE_CLANG_FRONTEND_H__
#define __TAN_INCLUDE_CLANG_FRONTEND_H__
#include "base.h"

namespace tanlang {
struct TanCompilation;
}

/**
 * \brief Call clang with commandline args
 * \details Requires the system to have `clang` in $PATH.
 *          This function doesn't directly invoke clang,
 *          but it uses the binary path to find paths to standard headers and libraries.
 *          (Try replacing $(which clang) with a blank text file and see if the compiler works :D)
 *  Defined in clang_driver.cpp
 */
extern int clang_main(int argc, char **argv);

/**
 * \brief Compile CXX files to objects using clang::driver::Driver
 * \details Only the following options in TanCompilation will affect the compilation of the files
 *  - TanCompilation::opt_level
 *  - TanCompilation::import_dirs
 * \note This function requires the system to have clang executable installed
 * \return Error code returned by clang::driver::Driver::ExecuteCompilation
 * */
int clang_compile(vector<str> input_files, tanlang::TanCompilation *config);

#endif /* __TAN_INCLUDE_CLANG_FRONTEND_H__ */
