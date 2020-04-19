#include <gflags/gflags.h>
#include "libtanc.h"
#include "base.h"
DEFINE_bool(print_ir_code, false, "print out llvm IR code if true");
DEFINE_bool(print_ast, false, "print out abstract syntax tree if true");
DEFINE_string(type, "exe", "output type, can be obj, exe, slib, or dlib");
DEFINE_string(o, "a.out", "output filename");
DEFINE_string(l, "", "files to link against");

int main(int argc, char **argv) {
  gflags::SetUsageMessage("tan compiler");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  std::vector<char *> files;
  files.reserve((size_t) argc);
  for (int i = 1; i < argc; ++i) {
    files.push_back(argv[i]);
  }
  std::vector<std::string> link_files = flag_to_list(FLAGS_l);
  std::vector<const char *> link_files_c;
  for (const auto &lf : link_files) {
    link_files_c.push_back(lf.c_str());
  }
  TanCompilation config;
  config.type = EXE;
  config.out_file = FLAGS_o.c_str();
  config.verbose = 0;
  if (link_files_c.empty()) {
    config.link_files = nullptr;
    config.n_link_files = 0;
  } else {
    config.n_link_files = link_files_c.size();
    config.link_files = link_files_c.data();
  }
  if (FLAGS_type == "obj") {
    config.type = OBJ;
  } else if (FLAGS_type == "slib") {
    config.type = SLIB;
  } else if (FLAGS_type == "dlib") {
    config.type = DLIB;
  }
  if (FLAGS_print_ast) {
    config.verbose = 2;
  } else if (FLAGS_print_ir_code) {
    config.verbose = 1;
  }
  compile_files((unsigned) files.size(), files.data(), &config);
  return 0;
}
