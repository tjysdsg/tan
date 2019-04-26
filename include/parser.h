#ifndef __TAN_PARSER_PARSER_H__
#define __TAN_PARSER_PARSER_H__
#include <cctype>
#include <iostream>
#include <cstdint>
#include "base.h"

namespace tanlang {

    struct ast_node;
    class Lexer;

    class Parser final {
      public:
        Parser() = default;
        ~Parser();
        void parse(Lexer const *lexer);
        ast_node *get_ast() const;

#ifdef DEBUG_ENABLED
        void print_ast_node(ast_node *n, int indent = 0);
#endif
      private:
        ast_node *_ast = nullptr;
    };
} // namespace tanlang
#endif // __TAN_PARSER_PARSER_H__
