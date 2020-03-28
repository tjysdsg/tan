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
  TanCompilation config;
  // TODO: create options for these
  config.type = OBJ;
  config.out_file = "a.out";
  config.verbose = 0;
  if (FLAGS_print_ast) {
    config.verbose = 2;
  } else if (FLAGS_print_ir_code) {
    config.verbose = 1;
  }
  compile_files((unsigned) files.size(), files.data(), &config);
  return 0;
}
