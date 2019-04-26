#include <gtest/gtest.h>
#include "reader_test.h"
#include "lexer_test.h"
#include "parser_test.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
