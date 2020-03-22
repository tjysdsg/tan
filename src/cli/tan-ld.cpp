#include <gflags/gflags.h>
#include "linker.h"
#include "base.h"

DEFINE_string(o, "a.out", "output file");

int main(int argc, char **argv) {
  --argc;
  ++argv;
  gflags::SetUsageMessage("tan linker");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  using tanlang::Linker;
  Linker linker;
  linker.add_flag("-o" + FLAGS_o);
  for (int i = 0; i < argc; ++i) {
    linker.add_file(std::string(argv[i]));
  }
  // TODO: other flags
  if (!linker.link()) { return 1; }
  return 0;
}
