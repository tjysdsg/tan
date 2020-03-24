#include <gflags/gflags.h>
#include "libtanc.h"
DEFINE_bool(print_ir_code, false, "print out llvm IR code if true");
DEFINE_bool(print_ast, false, "print out abstract syntax tree if true");

int main(int argc, char **argv) {
  gflags::SetUsageMessage("tan compiler");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::vector<char *> files;
  files.reserve((size_t) argc);
  for (int i = 1; i < argc; ++i) {
    files.push_back(argv[i]);
  }
  compile_files((unsigned) files.size(), files.data(), FLAGS_print_ast, FLAGS_print_ir_code);
  return 0;
}
