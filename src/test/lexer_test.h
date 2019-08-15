#ifndef TAN_LEXER_TEST_H
#define TAN_LEXER_TEST_H
#include "lexer.h"
#include "reader.h"
#include <gtest/gtest.h>

TEST(tokenize, line_comment) {
	std::string code = "// this is a comment";
	using tanlang::Reader;
	using tanlang::tokenize;
	using tanlang::TokenType;
	Reader r;
	r.from_string(code);
	auto result = tokenize(&r);
	EXPECT_EQ(result.size(), 1);
	EXPECT_EQ(result[0]->type, TokenType::COMMENTS);
	EXPECT_EQ(result[0]->value, " this is a comment");
}

TEST(tokenize, block_comment) {
	std::string code = "/* this is a comment */";
	using tanlang::Reader;
	using tanlang::tokenize;
	using tanlang::TokenType;
	Reader r;
	r.from_string(code);
	auto result = tokenize(&r);
	EXPECT_EQ(result.size(), 1);
	EXPECT_EQ(result[0]->type, TokenType::COMMENTS);
	EXPECT_EQ(result[0]->value, " this is a comment ");
}
#endif /* TAN_LEXER_TEST_H */
