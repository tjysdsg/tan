#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/expr.h"
#include "src/ast/decl.h"

using namespace tanlang;

size_t ParserImpl::parse_func_decl(const ASTBasePtr &_p) {
  ptr<FunctionDecl> p = ast_cast<FunctionDecl>(_p);

  bool is_public = false;
  bool is_external = false;

  if (at(p->_start_index)->value == "fn") { /// "fn"
    ++p->_end_index;
  } else if (at(p->_start_index)->value == "pub") { /// "pub fn"
    is_public = true;
    p->_end_index = p->_start_index + 2;
  } else if (at(p->_start_index)->value == "extern") { /// "extern"
    is_external = true;
    p->_end_index = p->_start_index + 2;
  } else {
    TAN_ASSERT(false);
  }

  /// function name
  // Don't use peek since it look ahead and returns ASTNodeType::FUNCTION when it finds "(",
  // but we only want the function name as an identifier
  // auto id = peek(p->_end_index);
  Token *id_token = at(p->_end_index);
  auto id = Identifier::Create(id_token->value);
  id->_start_index = id->_end_index = p->_end_index;
  id->set_token(p->get_token());
  if (id->get_node_type() != ASTNodeType::ID) {
    error(p->_end_index, "Invalid function name");
  }
  p->_end_index = parse_node(id);
  p->set_name(id->get_name());

  peek(p->_end_index, TokenType::PUNCTUATION, "(");
  ++p->_end_index;

  /// arguments
  vector<str> arg_names{};
  vector<ASTTypePtr> arg_types{};
  if (at(p->_end_index)->value != ")") {
    while (!eof(p->_end_index)) {
      auto arg = ArgDecl::Create();
      arg->set_token(at(p->_end_index));
      arg->_end_index = arg->_start_index = p->_end_index;
      p->_end_index = parse_node(arg);

      arg_names.push_back(arg->get_name());
      arg_types.push_back(arg->get_type());

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

  p->set_arg_names(arg_names);
  p->set_arg_types(arg_types);

  /// function return type
  auto ret_type = peek(p->_end_index);
  if (ret_type->get_node_type() != ASTNodeType::TY) {
    error(p->_end_index, "Expect a type");
  }
  p->_end_index = parse_node(ret_type);
  p->set_ret_type(ast_must_cast<ASTType>(ret_type));

  /// body
  if (!is_external) {
    auto body = peek(p->_end_index, TokenType::PUNCTUATION, "{");
    p->_end_index = parse_node(body);
    p->set_body(expect_stmt(body));
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
    ptr<Expr> arg = expect_expression(_arg);
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
