#ifndef __TAN_LEXER_LEXER_H__
#define __TAN_LEXER_LEXER_H__
namespace tanlang {
enum class TOKEN_TYPE {
    IF = 0,
    ELSE = 1 << 1,
    COMPARISON = 1 << 2,
    ID = 1 << 3,
    NUM = 1 << 4,
    LITERAL = 1 << 5,
    SYMBOL = 1 << 6,
};

}
#endif  // __TAN_LEXER_LEXER_H__
