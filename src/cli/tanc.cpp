#include <gflags/gflags.h>
#include "src/lib/libtanc.h"
#include "base.h"
DEFINE_string(files, "main.tan", "comma-separated list of files to compile");
DEFINE_bool(print_ir_code, false, "print out llvm IR code if true");
DEFINE_bool(print_ast, false, "print out abstract syntax tree if true");

int main(int argc, char **argv) {
  gflags::SetUsageMessage("tan compiler");
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  auto files = flag_to_list(FLAGS_files);
  for (auto f : files) {
    compile_file(f.c_str(), FLAGS_print_ast, FLAGS_print_ir_code);
  }
  return 0;
}
