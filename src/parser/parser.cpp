#include "parser.h"
#include "base.h"
#include "compiler_session.h"
#include "src/parser/parser_impl.h"

using namespace tanlang;

Parser::Parser(vector<Token *> tokens, str filename, CompilerSession *cs) : _impl(new ParserImpl(tokens,
    filename,
    cs)) {}

ASTBase *Parser::parse() {
  return _impl->parse();
}

str Parser::get_filename() const {
  return _impl->get_filename();
}
