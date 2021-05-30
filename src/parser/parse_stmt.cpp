#include "src/parser/parser_impl.h"
#include "src/ast/stmt.h"

using namespace tanlang;

size_t ParserImpl::parse_program(const ASTBasePtr &_p) {
  ptr<Program> p = ast_must_cast<Program>(_p);
  while (!eof(p->_end_index)) {
    auto stmt = Stmt::Create();
    stmt->set_token(at(p->_end_index));
    stmt->_start_index = p->_end_index;
    p->_end_index = parse_node(stmt);
    p->append_child(stmt);
  }
  return p->_end_index;
}

size_t ParserImpl::parse_stmt(const ASTBasePtr &_p) {
  StmtPtr p = ast_must_cast<Stmt>(_p);
  if (at(p->_end_index)->value == "{") { /// compound statement
    ++p->_end_index; /// skip "{"
    while (!eof(p->_end_index)) {
      auto node = peek(p->_end_index);
      while (node) { /// stops at a terminal token
        p->append_child(next_expression(p->_end_index, PREC_LOWEST));
        node = peek(p->_end_index);
      }
      if (at(p->_end_index)->value == "}") {
        ++p->_end_index; /// skip "}"
        break;
      }
      ++p->_end_index;
    }
  } else { /// single statement
    auto node = peek(p->_end_index);
    while (node) { /// stops at a terminal token
      p->append_child(next_expression(p->_end_index, PREC_LOWEST));
      node = peek(p->_end_index);
    }
    ++p->_end_index; /// skip ';'
  }
  return p->_end_index;
}
