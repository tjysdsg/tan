#include <gflags/gflags.h>
#include "libtanc.h"

DEFINE_string(o, "a.out", "output file");

int main(int argc, char **argv) {
  --argc;
  ++argv;
  gflags::SetUsageMessage("tan linker");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  TanCompilation config;
  // TODO: add cli options for these
  config.type = EXE;
  config.out_file = "a.out";
  tan_link((unsigned) argc, argv, &config);
  return 0;
}
