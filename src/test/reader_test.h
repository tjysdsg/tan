#ifndef __TAN_TEST_READER_TEST_H__
#define __TAN_TEST_READER_TEST_H__
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include "reader.h"

TEST(Reader, test1) {
    tanlang::Reader ra;
    ra.open("test_program.tan");
    EXPECT_EQ(ra.get_filename(), "test_program.tan");
    EXPECT_EQ(ra.next_line(), "#build_msg: str;");
    EXPECT_EQ(ra.get_line(4), "println(\"Building version {} ...\", version);");
    EXPECT_EQ(ra.next_line(), "prog_version = version;");
}
#endif // __TAN_TEST_READER_TEST_H__
