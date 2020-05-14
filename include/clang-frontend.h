#ifndef __TAN_INCLUDE_CLANG_FRONTEND_H__
#define __TAN_INCLUDE_CLANG_FRONTEND_H__

struct TanCompilation;

/**
 * \brief Clang main function
 * */
int clang_main(int argc, const char **argv);

/**
 * \brief Compile CXX files to objects using clang::driver::Driver
 * \details Only the following options in TanCompilation will affect the compilation of the files
 *  - TanCompilation::opt_level
 *  - TanCompilation::import_dirs
 * \note This function requires the system to have clang executable installed
 * \return Error code returned by clang::driver::Driver::ExecuteCompilation
 * */
int clang_compile(std::vector<const char *> input_files, TanCompilation *config);

#endif /* __TAN_INCLUDE_CLANG_FRONTEND_H__ */
