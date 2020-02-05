#include "lexer_test.h"
#include "reader_test.h"
#include "parser_test.h"
#include "codegen_test.h"
#include <gtest/gtest.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
