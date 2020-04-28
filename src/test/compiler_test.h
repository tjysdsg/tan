#ifndef __TAN_SRC_TEST_COMPILER_TEST_H__
#define __TAN_SRC_TEST_COMPILER_TEST_H__
#include "cli.h"
#include <gtest/gtest.h>
#include <iostream>

#define DEFINE_TEST(name, src)                                        \
  TEST(compiler, name) {                                              \
    const char *tmp[] = {"tanc", src, "-l", "runtime/runtime.so"};   \
    char ***cmd = (char ***) tmp;                                     \
    int argc = 4;                                                     \
    EXPECT_EQ(0, cli_main(&argc, (char ***) &cmd));                   \
  }

// FIXME: create multiple instances of LLVM causes segfault
// DEFINE_TEST(_1, "src/test/test_src/arithmetic.tan")
// DEFINE_TEST(_2, "src/test/test_src/array.tan")
// DEFINE_TEST(_3, "src/test/test_src/control_flow.tan")
// DEFINE_TEST(_4, "src/test/test_src/function.tan")
// DEFINE_TEST(_5, "src/test/test_src/intrinsics.tan")
// DEFINE_TEST(_6, "src/test/test_src/loop.tan")
// DEFINE_TEST(_7, "src/test/test_src/string.tan")
// DEFINE_TEST(_8, "src/test/test_src/struct.tan")
// DEFINE_TEST(_9, "src/test/test_src/type_system.tan")

#endif /* __TAN_SRC_TEST_COMPILER_TEST_H__ */
