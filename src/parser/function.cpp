#include "src/ast/ast_expr.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

size_t ASTFunction::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip "fn"
  _children.push_back(nullptr); /// function return type, set later
  _children.push_back(parser->parse<ASTType::ID>(_end_index, true)); /// function name
  parser->peek(_end_index, TokenType::PUNCTUATION, "(");
  ++_end_index;
  /// if the argument list isn't empty
  if (parser->at(_end_index)->value != ")") {
    while (!parser->eof(_end_index)) {
      std::shared_ptr<ASTNode> arg = std::make_shared<ASTArgDecl>(parser->at(_end_index), _end_index);
      _end_index = arg->parse(parser);
      _children.push_back(arg);
      if (parser->at(_end_index)->value == ",") {
        ++_end_index;
      } else { break; }
    }
  }
  parser->peek(_end_index, TokenType::PUNCTUATION, ")");
  ++_end_index;
  parser->peek(_end_index, TokenType::PUNCTUATION, ":");
  ++_end_index;
  _children[0] = parser->parse<ASTType::TY>(_end_index, true); /// return type

  /// get function body if exists, otherwise it's a external function
  auto *token = parser->at(_end_index);
  if (token->value == "{") {
    auto code = parser->peek(_end_index);
    _end_index = code->parse(parser);
    _children.push_back(code);
    _is_external = false;
  } else {
    _is_external = true;
  }
  return _end_index;
}

size_t ASTFunctionCall::nud(Parser *parser) {
  if (_parsed) { return _end_index; }
  _end_index = _start_index + 1; /// skip function name
  auto *token = parser->at(_end_index);
  if (token->value != "(") {
    report_code_error(token, "Invalid function call");
  }
  ++_end_index;
  while (!parser->eof(_end_index)) {
    _children.push_back(parser->next_expression(_end_index));
    if (parser->at(_end_index)->value == ",") { /// skip ,
      ++_end_index;
    } else { break; }
  }
  parser->peek(_end_index, TokenType::PUNCTUATION, ")");
  ++_end_index;
  _parsed = true;
  return _end_index;
}

} // namespace tanlang
