#include "cli.h"
#include "libtanc.h"
#include "base.h"
#include <gflags/gflags.h>
DEFINE_bool(print_ir_code, false, "print out llvm IR code if true");
DEFINE_bool(print_ast, false, "print out abstract syntax tree if true");
DEFINE_string(type, "exe", "output type, can be obj, exe, slib, or dlib");
DEFINE_string(o, "a.out", "output filename");
DEFINE_string(l, "", "files to link against");

int cli_main(int *pargc, char ***pargv) {
  gflags::SetUsageMessage("tan compiler");
  gflags::SetVersionString(std::to_string(TAN_VERSION_MAJOR) + "." + std::to_string(TAN_VERSION_MINOR) + "."
      + std::to_string(TAN_VERSION_PATCH));
  gflags::ParseCommandLineFlags(pargc, pargv, true);

  /// source files
  std::vector<char *> files;
  files.reserve((size_t) *pargc);
  for (int i = 1; i < *pargc; ++i) { files.push_back((*pargv)[i]); }
  /// files to link to
  std::vector<std::string> link_files = flag_to_list(FLAGS_l);
  std::vector<const char *> link_files_c;
  for (const auto &lf : link_files) { link_files_c.push_back(lf.c_str()); }

  /// build config
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

  /// output type
  if (FLAGS_type == "obj") { config.type = OBJ; }
  else if (FLAGS_type == "slib") { config.type = SLIB; }
  else if (FLAGS_type == "dlib") { config.type = DLIB; }

  /// verbosity
  if (FLAGS_print_ast) { config.verbose = 2; }
  else if (FLAGS_print_ir_code) { config.verbose = 1; }
  compile_files((unsigned) files.size(), files.data(), &config);
  return 0;
}

