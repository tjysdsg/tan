/**
 * \brief Tests for using tanc to compile and generate libraries
 */

#include "cli/cli.h"
#include "base.h"
#include <gtest/gtest.h>
#include <filesystem>

namespace fs = std::filesystem;

#ifndef TANC_PATH
#error "Define TANC_PATH before compiling this"
#endif
#ifndef TAN_PROJECT_SOURCE_DIR
#error "Define TAN_PROJECT_SOURCE_DIR before compiling this"
#endif
#ifndef TAN_TEST_SOURCE_DIR
#error "Define TAN_TEST_SOURCE_DIR before compiling this"
#endif

class MyFixture : public ::testing::Test {};

class TestCompileLib : public MyFixture {
private:
  bool _shared = false;

public:
  TestCompileLib(bool shared) : _shared(shared) {}

  void build_lib() {
    // remove previous outputs
    fs::remove(fs::path(__STR__(TAN_PROJECT_SOURCE_DIR) "/libtest.so"));
    fs::remove(fs::path(__STR__(TAN_PROJECT_SOURCE_DIR) "/libtest.a"));

    vector<const char *> cmd{
        __STR__(TANC_PATH),
        "-I" __STR__(TAN_PROJECT_SOURCE_DIR),
        "-L" __STR__(TAN_PROJECT_SOURCE_DIR) "/runtime",
        "-lruntime",
        _shared ? "-shared" : "-static",
        "-o", //
        _shared ? __STR__(TAN_PROJECT_SOURCE_DIR) "/libtest.so" : __STR__(TAN_PROJECT_SOURCE_DIR) "/libtest.a",
        __STR__(TAN_TEST_SOURCE_DIR) "/lib1.tan",
        __STR__(TAN_TEST_SOURCE_DIR) "/lib2.tan",
    };
    for (auto *c : cmd) {
      std::cout << c << " ";
    }
    std::cout << '\n';
    int argc = static_cast<int>(cmd.size());
    auto *argv = (char **)cmd.data();
    ASSERT_EQ(cli_main(argc, argv), 0);
  }

  void TestBody() override { build_lib(); }
};

// usage: test_compile_lib true/false
int main(int argc, char **argv) {
  assert(argc == 2);
  ::testing::InitGoogleTest(&argc, argv);

  bool shared_lib = str(argv[1]) == "true";
  ::testing::RegisterTest("tanc_test_fixture", "lib_test", nullptr, "", __FILE__, __LINE__,
                          [=]() -> MyFixture * { return new TestCompileLib(shared_lib); });

  return RUN_ALL_TESTS();
}
