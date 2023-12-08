#include "tan/tan.h"
#include "llvm_api/clang_frontend.h"
#include <llvm/Support/CommandLine.h>

using tanlang::TanCompilation;

int clang_compile(vector<str> input_files, TanCompilation *config) {
  vector<const char *> args;
  size_t n_import = config->import_dirs.size();
  args.reserve(input_files.size() + 2 * +1);
  args.push_back("clang");
  std::for_each(input_files.begin(), input_files.end(), [&args](const auto &s) { args.push_back(s.c_str()); });
  for (size_t i = 0; i < n_import; ++i) {
    args.push_back("-I");
    args.push_back(config->import_dirs[i].c_str());
  }
  str opt_level = opt_level_to_string(config->opt_level);
  args.push_back(opt_level.c_str());
  args.push_back("-c");

  return clang_main((int)args.size(), (char **)args.data());
}
