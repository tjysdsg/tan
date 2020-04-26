#include "linker.h"
#include <string>
#include <vector>
#include "clang-frontend.h"
#include <iostream>

namespace tanlang {

Linker::Linker() {
  _exe = "clang";
}

void Linker::add_files(std::vector<std::string> filenames) {
  _input_files.insert(_input_files.end(), filenames.begin(), filenames.end());
}

bool Linker::link() {
  std::vector<std::string> args{};
  args.insert(args.end(), _exe);
  args.insert(args.end(), "-cc");
  args.insert(args.end(), "-stdlib=libc++"); /// link to libc++ by default
  args.insert(args.end(), _input_files.begin(), _input_files.end());
  args.insert(args.end(), _flags.begin(), _flags.end());
  std::vector<const char *> cargs{};
  for (const auto &a : args) {
    if (a.length() == 0) { continue; }
    cargs.push_back(a.c_str());
    std::cout << a << " ";
  }
  std::cout << '\n';
  clang_main(static_cast<int>(cargs.size()), cargs.data());
  return true;
}

void Linker::add_flag(std::string flag) {
  _flags.push_back(flag);
}

void Linker::add_flags(std::vector<std::string> flags) {
  _flags.insert(_flags.begin(), flags.begin(), flags.end());
}

} // namespace tanlang
