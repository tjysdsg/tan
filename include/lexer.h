#ifndef TAN_LEXER_H
#define TAN_LEXER_H
#include <string>
#include <vector>
#include <iostream>

namespace tanlang {

struct line_info;
struct Token;

class Reader;

std::vector<Token *> tokenize(Reader *p_reader);

} // namespace tanlang

#endif /* TAN_LEXER_H */
