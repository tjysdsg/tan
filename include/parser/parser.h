#ifndef TAN_PARSER_H
#define TAN_PARSER_H
#include "base.h"
#include "ast/fwd.h"

namespace tanlang {

class ParserImpl;
class Token;
class Program;

/**
 * \brief Top Down Operator Precedence Parsing
 * \details A parser is bound to a specific tan source file. It does not care about any imported source files.
 */
class Parser final {
public:
  Parser() = delete;
  explicit Parser(SourceManager *ctx);
  ~Parser();
  Program *parse();

private:
  ParserImpl *_impl;
};

} // namespace tanlang

#endif /* TAN_PARSER_H */
