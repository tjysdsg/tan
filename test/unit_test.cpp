#include "parser_test.h"
#include "lexer_test.h"
#include "source_file_test.h"
#include <gtest/gtest.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
