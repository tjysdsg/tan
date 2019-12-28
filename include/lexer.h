#ifndef TAN_LEXER_H
#define TAN_LEXER_H

#include "lexdef.h"
#include "reader.h"
#include <string>
#include <vector>

namespace tanlang {
    struct Token {
        TokenType type = TokenType::END;
        std::string value = "";

        Token() = default;

        Token(TokenType tokenType, std::string value) : type(tokenType), value(std::move(value)) {}

        ~Token() = default;
    };

    struct line_info; // defined in reader.h
    std::vector<Token *> tokenize(Reader *p_reader, code_ptr start = code_ptr(0, 0));
} // namespace tanlang

#endif /* TAN_LEXER_H */
