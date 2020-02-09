#include <gflags/gflags.h>
#include "src/lib/libtanc.h"
DEFINE_string(files, "main.tan", "comma-separated list of files to compile");
DEFINE_bool(print_ir_code, false, "print out llvm IR code if true");
DEFINE_bool(print_ast, false, "print out abstract syntax tree if true");

static std::vector<std::string> flag_to_list(const std::string &flag, const std::string delimiter = ",") {
  size_t last = 0;
  size_t next = 0;
  std::vector<std::string> results;
  while ((next = flag.find(delimiter, last)) != std::string::npos) {
    results.push_back(flag.substr(last, next - last));
    last = next + 1;
  }
  results.push_back(flag.substr(last));
  return results;
}

int main(int argc, char **argv) {
  gflags::SetUsageMessage("tan compiler");
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  auto files = flag_to_list(FLAGS_files);
  for (auto f : files) {
    compile_file(f.c_str(), FLAGS_print_ast, FLAGS_print_ir_code);
  }
  return 0;
}
