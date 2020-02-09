#ifndef TAN_SRC_LIB_LIBTANC_H_
#define TAN_SRC_LIB_LIBTANC_H_

extern "C" {
bool compile_file(const char *input_path, bool print_ast, bool print_ir_code);
bool evaluate_file(const char *input_path, bool print_ast, bool print_ir_code);
}

#endif /* TAN_SRC_LIB_LIBTANC_H_ */
