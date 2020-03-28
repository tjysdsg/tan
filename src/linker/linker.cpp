#include "linker.h"
#include <string>
#include <vector>
#include "llvm/Support/Program.h"
#include "clang-frontend.h"
#include <string.h>
#include <iostream>

static char *convert(const std::string &s) {
  char *pc = new char[s.size() + 1];
  std::strcpy(pc, s.c_str());
  return pc;
}

namespace tanlang {

void Linker::add_file(std::string filename) {
  _input_files.push_back(filename);
}

void Linker::add_files(std::vector<std::string> filenames) {
  _input_files.insert(_input_files.end(), filenames.begin(), filenames.end());
}

bool Linker::link() {
  std::vector<std::string> args{};
  args.insert(args.end(), _input_files.begin(), _input_files.end());
  args.insert(args.end(), _flags.begin(), _flags.end());
  args.insert(args.begin(), "clang");
  args.insert(args.begin() + 1, "-cc");
  std::vector<char *> cargs{};
  std::transform(args.begin(), args.end(), std::back_inserter(cargs), convert);
  for (auto *ca : cargs) {
    std::cout << ca << " ";
  }
  clang_main(static_cast<int>(cargs.size()), const_cast<const char **>(cargs.data()));
  return true;
}

void Linker::add_flag(std::string flag) {
  _flags.push_back(flag);
}

void Linker::add_flags(std::vector<std::string> flags) {
  _flags.insert(_flags.begin(), flags.begin(), flags.end());
}

void Linker::add_flags(int c, char **flags) {
  _flags.reserve(static_cast<size_t>(c));
  for (int i = 0; i < c; ++i) {
    _flags.push_back(std::string(flags[i]));
  }
}

} // namespace tanlang
