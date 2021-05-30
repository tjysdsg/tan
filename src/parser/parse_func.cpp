#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_func.h"
#include "src/ast/expr.h"
#include "src/ast/factory.h"
#include "compiler_session.h"

using namespace tanlang;

size_t ParserImpl::parse_func_decl(const ASTBasePtr &p) {
  auto pfn = ast_cast<ASTFunction>(p);
  if (at(p->_start_index)->value == "fn") { /// "fn"
    ++p->_end_index;
  } else if (at(p->_start_index)->value == "pub") { /// "pub fn"
    pfn->_is_public = true;
    p->_end_index = p->_start_index + 2;
  } else if (at(p->_start_index)->value == "extern") { /// "extern"
    pfn->_is_external = true;
    p->_end_index = p->_start_index + 2;
  } else {
    TAN_ASSERT(false);
  }

  /// function return type, set later
  p->append_child(nullptr);

  /// function name
  // Don't use peek since it look ahead and returns ASTNodeType::FUNCTION when it finds "(",
  // but we only want the function name as an identifier
  // auto id = peek(p->_end_index);
  Token *id_token = at(p->_end_index);
  auto id = ast_create_identifier(_cs, id_token->value);
  id->_start_index = id->_end_index = p->_end_index;
  if (id->get_node_type() != ASTNodeType::ID) {
    error(p->_end_index, "Invalid function name");
  }
  p->_end_index = parse_node(id);
  p->set_data(id->get_data<str>());

  /// arguments
  peek(p->_end_index, TokenType::PUNCTUATION, "(");
  ++p->_end_index;
  if (at(p->_end_index)->value != ")") {
    while (!eof(p->_end_index)) {
      auto arg = ast_create_arg_decl(_cs);
      arg->set_token(at(p->_end_index));
      arg->_end_index = arg->_start_index = p->_end_index;
      p->_end_index = parse_node(arg);
      p->append_child(arg);
      if (at(p->_end_index)->value == ",") {
        ++p->_end_index;
      } else {
        break;
      }
    }
  }
  peek(p->_end_index, TokenType::PUNCTUATION, ")");
  ++p->_end_index;
  peek(p->_end_index, TokenType::PUNCTUATION, ":");
  ++p->_end_index;

  /// function return type
  auto ret_type = peek(p->_end_index);
  if (ret_type->get_node_type() != ASTNodeType::TY) {
    error(p->_end_index, "Expect a type");
  }
  p->_end_index = parse_node(ret_type);
  p->set_child_at(0, ret_type);

  /// body
  if (!pfn->_is_external) {
    auto body = peek(p->_end_index, TokenType::PUNCTUATION, "{");
    p->_end_index = parse_node(body);
    p->append_child(body);
  }

  return p->_end_index;
}

size_t ParserImpl::parse_func_call(const ASTBasePtr &_p) {
  ptr<FunctionCall> p = ast_must_cast<FunctionCall>(_p);

  p->set_name(at(p->_end_index)->value); /// function name
  ++p->_end_index;

  // No need to check since '(' is what distinguish a function call from an identifier at the first place
  // auto *token = at(p->_end_index); if (token->value != "(") { error("Invalid function call"); }
  ++p->_end_index; /// skip (

  /// args
  vector<ptr<Expr>> args{};
  while (!eof(p->_end_index) && at(p->_end_index)->value != ")") {
    auto _arg = next_expression(p->_end_index, p->get_lbp());
    ptr<Expr> arg = nullptr;
    if (!_arg || !(arg = ast_cast<Expr>(_arg))) {
      error(p->_end_index, "Expect an expression");
    }
    args.push_back(arg);

    if (at(p->_end_index)->value == ",") { /// skip ,
      ++p->_end_index;
    } else { // TODO: remove this else?
      break;
    }
  }

  peek(p->_end_index, TokenType::PUNCTUATION, ")");
  ++p->_end_index;

  return p->_end_index;
}
