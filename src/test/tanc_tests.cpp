#include "cli.h"
#include "base.h"
#include <gtest/gtest.h>

#ifndef TAN_SOURCE
#error "Define TAN_SOURCE before compiling this"
#endif

TEST(tanc, general) {
  std::vector<const char *>
      cmd{"tanc", "--print-ast=true", "--print-ir-code=true", __STR__(TAN_SOURCE), "-l", "runtime/runtime.so"};
  int argc = static_cast<int>(cmd.size());
  auto *argv = c_cast(char**, cmd.data());
  EXPECT_EQ(0, cli_main(&argc, &argv));
  EXPECT_EQ(0, system("./a.out"));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
