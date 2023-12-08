#include "linker/linker.h"
#include <string>
#include "llvm_api/clang_frontend.h"

namespace tanlang {

void Linker::add_files(const vector<str> &filenames) {
  _input_files.insert(_input_files.end(), filenames.begin(), filenames.end());
}

bool Linker::link() {
  // TODO: allow changing these default options
  vector<const char *> args{};
  args.push_back("clang");
  args.push_back("-stdlib=libc++"); /// link to libc++ by default
  printf("clang -stdlib=libc++ ");

  for (const str &e : _input_files) {
    args.push_back(e.c_str());
    printf("%s ", e.c_str());
  }
  for (const str &e : _flags) {
    args.push_back(e.c_str());
    printf("%s ", e.c_str());
  }
  args.push_back("-lm"); /// link to libm by default
  printf("-lm\n");

  return !clang_main((int)args.size(), (char **)args.data());
}

void Linker::add_flag(const str &flag) { _flags.push_back(flag); }

void Linker::add_flags(vector<str> flags) { _flags.insert(_flags.begin(), flags.begin(), flags.end()); }

} // namespace tanlang
