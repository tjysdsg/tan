#ifndef TAN_PARSER_TEST_H
#define TAN_PARSER_TEST_H

#include "parser.h"
#include "lexdef.h"
#include <gtest/gtest.h>
#include <iostream>

using tanlang::Reader;
using tanlang::tokenize;
using tanlang::TokenType;
using tanlang::Parser;

TEST(parser, arithmatic) {
    std::string code = "1 + 2 * 3";
    Reader r;
    r.from_string(code);
    auto tokens = tokenize(&r);

    Parser p(tokens);
    p.parse();
    p._root->printTree();
}

#endif /* TAN_PARSER_TEST_H */
