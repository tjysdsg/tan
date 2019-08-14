#ifndef TAN_LEXER_TEST_H
#define TAN_LEXER_TEST_H
#include "lexer.h"
#include "reader.h"

TEST(tokenize, test1) {
    std::string code = "//this is a comment";
    tanlang::line_info l(0);
    l.code = code;
    std::vector<tanlang::rule> rules;
    rules.emplace_back(tanlang::rule(R"(//.*)", tanlang::TokenType::COMMENTS));
    auto tokens = tanlang::tokenize(&l, rules);
    EXPECT_EQ(tokens.size(), 1);
    EXPECT_EQ(tokens[0]->type, tanlang::TokenType::COMMENTS);
    EXPECT_EQ(tokens[0]->value, "//this is a comment");
}
#endif /* TAN_LEXER_TEST_H */
