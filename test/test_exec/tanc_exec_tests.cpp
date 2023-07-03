/**
 * \brief Tests for using tanc to compile and generate executable
 */

#include "cli/cli.h"
#include "base.h"
#include <gtest/gtest.h>
#include <utility>
#include <filesystem>

#ifndef TAN_PROJECT_SOURCE_DIR
#error "Define TAN_PROJECT_SOURCE_DIR before compiling this"
#endif
#ifndef TANC_PATH
#error "Define TANC_PATH before compiling this"
#endif
#ifndef TAN_TEST_SOURCE_DIR
#error "Define TAN_TEST_SOURCE_DIR before compiling this"
#endif

namespace fs = std::filesystem;

struct TestConfig {
  str filename;
  int expected_compilation_return_value = 0;
  int expected_run_return_value = 0;
  str tmp_dir = "out";
  str output_file = "a.out";

  vector<str> flags{};
};

class TanCExecTests : public ::testing::Test {
private:
  TestConfig _config{};

public:
  TanCExecTests(TestConfig config) : _config(std::move(config)) {}

  void TestBody() override {
    vector<const char *> cmd{__STR__(TANC_PATH),
                             "-I" __STR__(TAN_PROJECT_SOURCE_DIR),
                             "-L" __STR__(TAN_PROJECT_SOURCE_DIR) "/runtime",
                             "-lruntime",
                             _config.filename.c_str(),
                             "-o",
                             _config.output_file.c_str()};
    for (auto *c : cmd) {
      std::cout << c << " ";
    }
    std::cout << '\n';

    // compile
    int argc = static_cast<int>(cmd.size());
    auto *argv = (char **)cmd.data();
    ASSERT_EQ(_config.expected_compilation_return_value, cli_main(argc, argv));

    // run if compilation succeeded
    if (0 == _config.expected_compilation_return_value) {
      EXPECT_EQ(_config.expected_run_return_value, system(_config.output_file.c_str()));
    }
  }
};

vector<str> opt_levels{"", "-g", "-O0", "-O1", "-O2", "-O3"};

// usage: tanc_exec_tests.cpp xxx.tan <expected_compilation_return_value> output_dir
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  assert(argc == 4);

  fs::path file(argv[1]);
  int expected_compilation_return_value = std::stoi(argv[2]);
  fs::path output_dir(argv[3]);

  if (!fs::exists(output_dir)) {
    fs::create_directory(output_dir);
  }

  str filename = std::filesystem::path(file).filename();

  for (const auto &opt : opt_levels) {
    str test_name = fmt::format("test_{}_{}", filename, opt);
    str output_file = output_dir / (test_name + ".out");

    TestConfig config{file, expected_compilation_return_value, 0, output_dir, output_file, {opt}};

    ::testing::RegisterTest("tanc_exec_tests", test_name.c_str(), nullptr, file.c_str(), __FILE__, __LINE__,
                            [=]() { return new TanCExecTests(config); });
  }

  return RUN_ALL_TESTS();
}
