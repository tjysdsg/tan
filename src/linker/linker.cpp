#include "linker.h"
#include <string>
#include <vector>
#include "clang-frontend.h"
#include <iostream>

namespace tanlang {

Linker::Linker() : _exe("clang") {}

void Linker::add_files(vector<str> filenames) {
  _input_files.insert(_input_files.end(), filenames.begin(), filenames.end());
}

bool Linker::link() {
  vector<str> args{};
  args.push_back(_exe);
  args.push_back("-stdlib=libc++"); /// link to libc++ by default
  args.insert(args.end(), _input_files.begin(), _input_files.end());
  args.insert(args.end(), _flags.begin(), _flags.end());
  args.push_back("-lm"); /// link to libm by default

  vector<const char *> cargs{};
  for (const auto &a: args) {
    if (a.length() == 0) { continue; }
    cargs.push_back(a.c_str());
    std::cout << a << " ";
  }
  std::cout << '\n';
  return !clang_main(static_cast<int>(cargs.size()), cargs.data());
}

void Linker::add_flag(const str &flag) {
  _flags.push_back(flag);
}

void Linker::add_flags(vector<str> flags) {
  _flags.insert(_flags.begin(), flags.begin(), flags.end());
}

} // namespace tanlang
