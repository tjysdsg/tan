#ifndef TAN_PARSER_H
#define TAN_PARSER_H
#include "base.h"
#include "src/ast/fwd.h"
#include <memory>
#include <stack>

namespace tanlang {

class ParserImpl;
class Token;

/**
 * \brief Top Down Operator Precedence Parsing
 */
class Parser final {
public:
  Parser() = delete;
  explicit Parser(ASTContext *ctx);
  ~Parser();
  ASTBase *parse();

private:
  ParserImpl *_impl;
};

} // namespace tanlang

#endif /* TAN_PARSER_H */
