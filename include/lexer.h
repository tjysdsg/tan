#ifndef TAN_LEXER_H
#define TAN_LEXER_H

#include "token.h"
#include "reader.h"
#include <string>
#include <vector>
#include <iostream>

namespace tanlang {
struct line_info; // defined in reader.h
std::vector<Token *> tokenize(Reader *p_reader, code_ptr start = code_ptr(0, 0));
} // namespace tanlang

#endif /* TAN_LEXER_H */
