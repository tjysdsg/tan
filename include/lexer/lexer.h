#ifndef TAN_LEXER_H
#define TAN_LEXER_H
#include "base.h"
#include <iostream>

namespace tanlang {

class Token;
class SourceFile;

vector<Token *> tokenize(SourceFile *p_reader);

} // namespace tanlang

#endif /* TAN_LEXER_H */
