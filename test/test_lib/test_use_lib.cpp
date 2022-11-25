/**
 * \brief Tests for using tanc to compile and generate libraries
 */

#include "cli.h"
#include "base.h"
#include <gtest/gtest.h>

#ifndef TAN_PROJECT_SOURCE_DIR
#error "Define TAN_PROJECT_SOURCE_DIR before compiling this"
#endif
#ifndef TAN_TEST_SOURCE_DIR
#error "Define TAN_TEST_SOURCE_DIR before compiling this"
#endif

class MyFixture : public ::testing::Test {};

class TestUseLib : public MyFixture {
public:
  TestUseLib() {}

  void TestBody() override {
    // compile the executable that uses the library and run it
    vector<const char *> cmd = {__STR__(TAN_PROJECT_SOURCE_DIR)"/bin/tanc", "-I" __STR__(TAN_PROJECT_SOURCE_DIR),
        "-L" __STR__(TAN_PROJECT_SOURCE_DIR) "/runtime", "-lruntime", "-L" __STR__(TAN_PROJECT_SOURCE_DIR), "-ltest",
        "-o", "a.out", __STR__(TAN_TEST_SOURCE_DIR)"/cli.tan"};
    for (auto *c: cmd) {
      std::cout << c << " ";
    }
    std::cout << '\n';
    int argc = static_cast<int>(cmd.size());
    char **argv = (char **) cmd.data();
    ASSERT_EQ(cli_main(argc, argv), 0);
    EXPECT_EQ(system("./a.out"), 0);
  }
};

void register_tanc_test() {
  ::testing::RegisterTest("tanc_test_fixture",
      "lib_test",
      nullptr,
      "",
      __FILE__,
      __LINE__,
      [=]() -> MyFixture * { return new TestUseLib(); });
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  register_tanc_test();
  return RUN_ALL_TESTS();
}
