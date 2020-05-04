#ifndef TAN_SRC_PARSER_TOKEN_CHECK_H_
#define TAN_SRC_PARSER_TOKEN_CHECK_H_

namespace tanlang {

struct Token;

bool check_typename_token(Token *token);
bool check_terminal_token(Token *token);
bool check_arithmetic_token(Token *token);

}

#endif /*TAN_SRC_PARSER_TOKEN_CHECK_H_*/
