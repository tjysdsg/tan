#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_type.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"
#include "src/ast/ast_base.h"
#include "src/ast/factory.h"
#include "compiler_session.h"
#include <fmt/core.h>

using namespace tanlang;

size_t ParserImpl::parse_ty_array(const ASTTypePtr &p) {
  bool done = false;
  while (!done) {
    /// current token should be "[" right now
    ++p->_end_index; /// skip "["

    /// subtype
    ASTTypePtr sub = make_ptr<ASTType>(*p);
    p->_tyty = Ty::ARRAY;
    p->_sub_types.clear();
    p->_sub_types.push_back(sub);

    /// size
    ASTBasePtr _size = peek(p->_end_index);
    if (_size->get_node_type() != ASTNodeType::INTEGER_LITERAL) {
      error(p->_end_index, "Expect an unsigned integer as the array size");
    }
    p->_end_index = parse_node(_size);

    auto size = ast_must_cast<IntegerLiteral>(_size);
    size_t array_size = size->get_value();
    if (static_cast<int64_t>(array_size) < 0) {
      error(p->_end_index, "Expect an unsigned integer as the array size");
    }

    p->_array_size = array_size;

    /// skip "]"
    peek(p->_end_index, TokenType::PUNCTUATION, "]");
    ++p->_end_index;

    /// if followed by a "[", this is a multi-dimension array
    if (at(p->_end_index)->value != "[") {
      done = true;
    }
  }
  return p->_end_index;
}

size_t ParserImpl::parse_ty(const ASTTypePtr &p) {
  while (!eof(p->_end_index)) {
    Token *token = at(p->_end_index);
    auto qb = ASTType::basic_tys.find(token->value);
    auto qq = ASTType::qualifier_tys.find(token->value);

    if (qb != ASTType::basic_tys.end()) { /// base types
      p->_tyty = TY_OR(p->_tyty, qb->second);
    } else if (qq != ASTType::qualifier_tys.end()) { /// TODO: qualifiers
      if (token->value == "*") { /// pointer
        auto sub = std::make_shared<ASTType>(*p);
        p->_tyty = Ty::POINTER;
        p->_sub_types.clear();
        p->_sub_types.push_back(sub);
      }
    } else if (token->type == TokenType::ID) { /// struct or enum
      *p = *(_cs->get_type(token->value));
      if (!p) {
        error(p->_end_index, "Invalid type name");
      }
    } else {
      break;
    }
    ++p->_end_index;
  }

  /// composite types
  Token *token = at(p->_end_index);
  if (token->value == "[") { /// array
    p->_end_index = parse_ty_array(p);
  }
  return p->_end_index;
}
