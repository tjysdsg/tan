/**
 * \brief Tests for using tanc to compile and generate executable
 */

#include "cli/cli.h"
#include "base.h"
#include <gtest/gtest.h>
#include <utility>

#ifndef TAN_PROJECT_SOURCE_DIR
#error "Define TAN_PROJECT_SOURCE_DIR before compiling this"
#endif
#ifndef TANC_PATH
#error "Define TANC_PATH before compiling this"
#endif
#ifndef TAN_TEST_SOURCE_DIR
#error "Define TAN_TEST_SOURCE_DIR before compiling this"
#endif

class TanCExecTests : public ::testing::Test {
private:
  str _filename;
  int _expected_compilation_return_value = 0;

public:
  TanCExecTests(str filename, int expected_compilation_return_value)
      : _filename(std::move(filename)), _expected_compilation_return_value(expected_compilation_return_value) {}

  void TestBody() override {
    vector<const char *> cmd{__STR__(TANC_PATH),
                             "-I" __STR__(TAN_PROJECT_SOURCE_DIR),
                             "-L" __STR__(TAN_PROJECT_SOURCE_DIR) "/runtime",
                             "-lruntime",
                             _filename.c_str(),
                             "-o",
                             "a.out"};
    for (auto *c : cmd) {
      std::cout << c << " ";
    }
    std::cout << '\n';

    // compile
    int argc = static_cast<int>(cmd.size());
    auto *argv = (char **)cmd.data();
    ASSERT_EQ(_expected_compilation_return_value, cli_main(argc, argv));

    // run if compilation succeeded
    if (0 == _expected_compilation_return_value) {
      EXPECT_EQ(0, system("./a.out"));
    }
  }
};

// usage: tanc_exec_tests.cpp xxx.tan <expected_compilation_return_value>
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  assert(argc == 3);

  str file(argv[1]);
  int expected_compilation_return_value = std::stoi(argv[2]);

  ::testing::RegisterTest("tanc_test_fixture", ("test_" + file).c_str(), nullptr, file.c_str(), __FILE__, __LINE__,
                          [=]() { return new TanCExecTests(file, expected_compilation_return_value); });

  return RUN_ALL_TESTS();
}
