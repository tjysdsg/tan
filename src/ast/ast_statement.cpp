#include "src/ast/ast_statement.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

ASTStatement::ASTStatement(bool is_compound, Token *t, size_t ti) : ASTNode(ASTType::STATEMENT,
    op_precedence[ASTType::STATEMENT],
    0,
    t,
    ti) { _is_compound = is_compound; }

ASTStatement::ASTStatement(Token *t, size_t ti) : ASTNode(ASTType::STATEMENT,
    op_precedence[ASTType::STATEMENT],
    0,
    t,
    ti) {}

size_t ASTStatement::nud() {
  _end_index = _start_index;
  if (_is_compound) { /// compound statement
    ++_end_index; /// skip "{"
    while (!_parser->eof(_end_index)) {
      auto node = _parser->peek(_end_index);
      while (node) { /// stops at a terminal token
        _children.push_back(_parser->next_expression(_end_index, 0));
        node = _parser->peek(_end_index);
      }
      if (_parser->at(_end_index)->value == "}") {
        ++_end_index; /// skip "}"
        break;
      }
      ++_end_index;
    }
  } else { /// single statement
    auto node = _parser->peek(_end_index);
    while (node) { /// stops at a terminal token
      _children.push_back(_parser->next_expression(_end_index, 0));
      node = _parser->peek(_end_index);
    }
    ++_end_index; /// skip terminal token
  }
  return _end_index;
}

} // namespace tanlang
