#ifndef TAN_TEST_READER_TEST_H
#define TAN_TEST_READER_TEST_H

#include "reader.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(Reader, open) {
  tanlang::Reader ra;
  ra.open("test_program.tan");
  EXPECT_EQ(ra.get_filename(), "test_program.tan");
  std::vector<std::string> lines = {"#build_msg: str;",
                                    "#prog_version: u32;",
                                    "#before_build() {",
                                    R"(println("Building version {u32} ...", prog_version);)",
                                    R"(println("{str}", build_msg);)",
                                    "}",
                                    "",
                                    "#after_build() {",
                                    "println(\"Success!!\");",
                                    "}",
                                    "",
                                    "fn main(argc: u32, argv: [[]]) : u32 {",
                                    "// get all arguments",
                                    "args: [[]] = Vec::from_buffer(Buffer::new(argc * sizeof(*str)));",
                                    "// this is redundant, but for illustration purposes",
                                    "for(i: u32 = 0; i < argc; i += 1) {",
                                    "args[i] = argv[i];",
                                    "}",
                                    "// parse arguments",
                                    "for(i: u32 = 0; i < argc; i += 1) {",
                                    R"(if (args[i] == "-v" || args[i] == "--version") {)",
                                    R"(println("Version: {}", #prog_version);)",
                                    "return 0;",
                                    "}",
                                    "",
                                    R"(if (args[i] == "-h" || args[i] == "--help") {)",
                                    R"(print("Avaiable options:\n");)",
                                    R"(println("use -v or --version see program version");)",
                                    "return 1;",
                                    "}",
                                    "}",
                                    "return 0;",
                                    "}"};
  size_t i = 0, j = 0;
  while (i < ra.size()) {
    EXPECT_EQ(ra[i].code, lines[j]);
    ++i;
    ++j;
  }
}

TEST(Reader, from_string1) {
  tanlang::Reader ra;
  ra.from_string("hi!\nthis is tan");
  EXPECT_EQ(ra.get_filename(), "");
  std::vector<std::string> lines = {"hi!", "this is tan"};
  EXPECT_EQ(ra.size(), lines.size());
  size_t i = 0, j = 0;
  while (i < ra.size()) {
    EXPECT_EQ(ra[i].code, lines[j]);
    ++i;
    ++j;
  }
}

TEST(Reader, from_string2) {
  tanlang::Reader ra;
  ra.from_string("hi!\nthis is tan\n");
  EXPECT_EQ(ra.get_filename(), "");
  std::vector<std::string> lines = {"hi!", "this is tan"};
  EXPECT_EQ(ra.size(), lines.size());
  size_t i = 0, j = 0;
  while (i < ra.size()) {
    EXPECT_EQ(ra[i].code, lines[j]);
    ++i;
    ++j;
  }
}

#endif // TAN_TEST_READER_TEST_H
