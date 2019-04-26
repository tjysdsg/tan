#ifndef __TAN_SRC_TEST_PARSER_TEST_H__
#define __TAN_SRC_TEST_PARSER_TEST_H__
#include "src/parser/parser_internal.h"
#include "parser.h"
#include "lexer.h"

TEST(Parser, parseIDENTIFIER) {
    tanlang::Lexer lx;
    tanlang::Parser ps;
    lx.read_string("shit fuck damn little bitch");
    lx.lex();
    ps.parse(&lx);
    ps.print_ast_node(ps.get_ast());
}

#endif // __TAN_SRC_TEST_PARSER_TEST_H__
