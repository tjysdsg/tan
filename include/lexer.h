#ifndef TAN_LEXER_H
#define TAN_LEXER_H
#include <string>
#include <utility>
#include <vector>
#include <regex>

namespace tanlang {
    enum class TokenType {
        END = -1,
        COMMENTS,
    };
    
    typedef std::pair<std::regex, TokenType> rule;

    struct token {
        TokenType type = TokenType::END;
        std::string value = "";
        token() = default;
        token(TokenType tokenType, std::string value)
            : type(tokenType), value(std::move(value)) {}
        ~token() = default;
    };

    struct line_info; // defined in include/reader.h
    std::vector<token *> tokenize(const line_info *const line,
                                  std::vector<rule> rules);
} // namespace tanlang

#endif /* TAN_LEXER_H */
