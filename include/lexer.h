#ifndef TAN_LEXER_H
#define TAN_LEXER_H
#include "lexdef.h"
#include "reader.h"
#include <string>
#include <vector>
namespace tanlang {
    struct token {
        TokenType type = TokenType::END;
        std::string value = "";
        token() = default;
        token(TokenType tokenType, std::string value) : type(tokenType), value(std::move(value)) {}
        ~token() = default;
    };

    struct line_info; // defined in include/reader.h
    std::vector<token *> tokenize(Reader *p_reader, code_ptr start = code_ptr(0, 0));
} // namespace tanlang

#endif /* TAN_LEXER_H */
