#include "linker.h"
#include <vector>
#include <string>
#include <stdexcept>

struct LinkJob {
  // TODO: make this portable
  std::vector<std::string> pre_options = {"--eh-frame-hdr", //
                                          "--hash-style=gnu", //
                                          "-m elf_x86_64", //
                                          "--dynamic-linker=/lib64/ld-linux-x86-64.so.2", //
                                          "-pie", //
                                          "/usr/lib/Scrt1.o", //
                                          "/usr/lib/crti.o", //
                                          "/usr/lib/gcc/x86_64-pc-linux-gnu/9.2.1/crtbeginS.o", //
                                          "-L/usr/lib/gcc/x86_64-pc-linux-gnu/9.2.1", //
                                          "-L/usr/lib", //
                                          "-L/lib", //
  };
  std::vector<std::string> post_options = {"-lstdc++", //
                                           "-lm", //
                                           "-lc", //
                                           "-lgcc_s", //
                                           "-lgcc", //
                                           "/usr/lib/gcc/x86_64-pc-linux-gnu/9.2.1/crtendS.o", //
                                           "/usr/lib/crtn.o", //
  };
  std::vector<std::string> user_options = {};

  std::string get_command() {
    std::string cmd = "ld ";
    for (auto op : pre_options) {
      cmd += op + " ";
    }
    for (auto op : user_options) {
      cmd += op + " ";
    }
    for (auto op : post_options) {
      cmd += op + " ";
    }
    return cmd;
  }
};

int main(int argc, char **argv) {
  LinkJob lj;
  if (argc > 1) {
    std::vector<std::string> args(argv + 1, argv + argc);
    lj.user_options = args;
    return system(lj.get_command().c_str());
  } else {
    throw std::runtime_error("Please specify arguments");
  }
}
