#ifndef TAN_TEST_READER_TEST_H
#define TAN_TEST_READER_TEST_H
#include <gtest/gtest.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include "reader.h"
#include "utils.hpp"

TEST(Reader, test1) {
    tanlang::Reader ra;
    ra.open("test_program.tan");
    EXPECT_EQ(ra.get_filename(), "test_program.tan");
    std::vector<std::string> lines = {
        "#build_msg: str;",
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
        "fn main(argc: u32, argv: [[]]) -> u32 {",
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
    // test reader's content and its iterator
    auto rs = ra.begin();
    auto re = ra.end();
    auto ls = lines.begin();
    auto le = lines.end();
    while (rs != re and ls != le) {
        EXPECT_EQ((*rs).code, *ls);
        ++rs;
        ++ls;
    }
}

TEST(Reader, test2) {
	tanlang::Reader ra;
	ra.from_string("hi!\nthis is tan\n");
	EXPECT_EQ(ra.get_filename(), "");
	std::vector<std::string> lines = {"hi!", "this is tan", ""};
	// test reader's content and its iterator
	auto rs = ra.begin();
	auto re = ra.end();
	auto ls = lines.begin();
	auto le = lines.end();
	while (rs != re and ls != le) {
		EXPECT_EQ((*rs).code, *ls);
		++rs;
		++ls;
	}
}
#endif // TAN_TEST_READER_TEST_H
