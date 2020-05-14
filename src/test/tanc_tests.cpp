#include "cli.h"
#include "base.h"
#include <gtest/gtest.h>

#ifndef TAN_SOURCE
#error "Define TAN_SOURCE before compiling this"
#endif

#ifndef TAN_PROJECT_SOURCE_DIR
#error "Define TAN_PROJECT_SOURCE_DIR before compiling this"
#endif

#ifndef TAN_TEST_NAME
#error "Define TAN_TEST_NAME before compiling this"
#endif

TEST(tanc, TAN_TEST_NAME) {
  vector<const char *> cmd
      {"tanc", "--print-ast", "--print-ir", "-I" __STR__(TAN_PROJECT_SOURCE_DIR), __STR__(TAN_SOURCE),
          "-lruntime/runtime.so"};
  int argc = static_cast<int>(cmd.size());
  auto *argv = c_cast(char**, cmd.data());
  EXPECT_EQ(0, cli_main(argc, argv));
  EXPECT_EQ(0, system("./a.out"));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
