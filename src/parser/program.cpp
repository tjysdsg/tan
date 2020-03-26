#include "src/ast/ast_expr.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

/**
 * \brief: parse a list of (compound) statements
 * */
size_t ASTProgram::nud(Parser *parser) {
  _end_index = _start_index;
  while (!parser->eof(_end_index)) {
    _children.push_back(parser->parse<ASTType::STATEMENT>(_end_index, true));
  }
  return _end_index;
}

}
