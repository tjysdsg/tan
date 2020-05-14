#ifndef __TAN_INCLUDE_CLANG_FRONTEND_H__
#define __TAN_INCLUDE_CLANG_FRONTEND_H__

struct TanCompilation;

/**
 * \brief Clang main function
 * */
int clang_main(int argc, const char **argv);

int clang_compile(std::vector<const char *> input_files, TanCompilation *config);

#endif /* __TAN_INCLUDE_CLANG_FRONTEND_H__ */
