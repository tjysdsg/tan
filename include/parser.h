#ifndef TAN_PARSER_H
#define TAN_PARSER_H
#include "base.h"
#include <memory>
#include <stack>

namespace tanlang {

class CompilerSession;
class ParserImpl;
struct Token;

/**
 * \brief Parser
 * \details Operator precedence parsing
 * */
class Parser final {
public:
  Parser() = delete;
  Parser(CompilerSession *cs);
  ~Parser();
  ASTBase *parse();

private:
  ParserImpl *_impl;
};

} // namespace tanlang

#endif /* TAN_PARSER_H */
