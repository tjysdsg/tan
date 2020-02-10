#ifndef TAN_SRC_LIB_LIBTANC_H_
#define TAN_SRC_LIB_LIBTANC_H_

extern "C" {
/**
 * \brief Compile a source file into an object file.
 * \details The output file is named as "<name of the souce file>.o" and it is at current working directory.
 *          If current build is release, all exceptions are captured and e.what() is printed out to stderr.
 *          If current build is debug, all exceptions are not captured, making debugging easier.
 * \param input_path the source file path, can be relative or absolute path.
 * \param print_ast print out to stdout Abstract Syntax Tree if true.
 * \param print_ir_code print out LLVM IR code to stderr if true.
 * \return If current build is release, returns true if no error occurred, and vice versa.
 *  If current build is debug, either returns true or doesn't return, because all errors are captured by
 *         a try-catch clause.
 * */
bool compile_file(const char *input_path, bool print_ast, bool print_ir_code);

/**
 * \brief Evaluate the main function in a source file.
 * \details The return value of the main function is printed to stdout.
 *          If current build is release, all exceptions are captured and e.what() is printed out to stderr.
 *          If current build is debug, all exceptions are not captured, making debugging easier.
 * \param input_path the source file path, can be relative or absolute path.
 * \param print_ast print out to stdout Abstract Syntax Tree if true.
 * \param print_ir_code print out LLVM IR code to stderr if true.
 * \return If current build is release, returns true if no error occurred, and vice versa.
 *  If current build is debug, either returns true or doesn't return, because all errors are captured by
 *         a try-catch clause.
 * */
bool evaluate_file(const char *input_path, bool print_ast, bool print_ir_code);

}

#endif /* TAN_SRC_LIB_LIBTANC_H_ */
