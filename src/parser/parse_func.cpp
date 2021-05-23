#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_func.h"
#include "src/ast/factory.h"
#include "compiler_session.h"

using namespace tanlang;

// TODO: move scope checking to analysis phase

size_t ParserImpl::parse_func_decl(const ParsableASTNodePtr &p) {
  auto pfn = ast_cast<ASTFunction>(p);
  if (at(p->_start_index)->value == "fn") {
    /// skip "fn"
    ++p->_end_index;
  } else if (at(p->_start_index)->value == "pub") {
    pfn->_is_public = true;
    /// skip "pub fn"
    p->_end_index = p->_start_index + 2;
  } else if (at(p->_start_index)->value == "extern") {
    pfn->_is_external = true;
    /// skip "pub fn"
    p->_end_index = p->_start_index + 2;
  } else { TAN_ASSERT(false); }

  /// function return type, set later
  p->append_child(nullptr);

  /// function name
  // Don't use peek since it look ahead and returns ASTType::FUNCTION when it finds "(",
  // but we only want the function name as an identifier
  // auto id = peek(p->_end_index);
  Token *id_token = at(p->_end_index);
  auto id = ast_create_identifier(_cs, id_token->value);
  id->_start_index = id->_end_index = p->_end_index;

  if (id->get_node_type() != ASTType::ID) { error(p->_end_index, "Invalid function name"); }
  p->_end_index = parse_node(id);
  p->set_data(id->get_data<str>()); /// name

  _cs->push_scope(); /// pop a new scope
  /// arguments
  peek(p->_end_index, TokenType::PUNCTUATION, "(");
  ++p->_end_index;
  if (at(p->_end_index)->value != ")") {
    while (!eof(p->_end_index)) {
      auto arg = ast_create_arg_decl(_cs);
      arg->set_token(at(p->_end_index));
      arg->_end_index = arg->_start_index = p->_end_index;
      p->_end_index = parse_node(arg); /// this will add args to the current scope
      p->append_child(arg);
      if (at(p->_end_index)->value == ",") {
        ++p->_end_index;
      } else { break; }
    }
  }
  peek(p->_end_index, TokenType::PUNCTUATION, ")");
  ++p->_end_index;
  peek(p->_end_index, TokenType::PUNCTUATION, ":");
  ++p->_end_index;

  /// function return type
  auto ret_type = peek(p->_end_index);
  p->_end_index = parse_ty(ret_type);
  p->set_child_at(0, ret_type);

  /// body
  if (!pfn->_is_external) {
    auto body = peek(p->_end_index, TokenType::PUNCTUATION, "{");
    p->_end_index = parse_node(body);
    p->append_child(body);
  }
  _cs->pop_scope(); /// pop function scope

  return p->_end_index;
}

size_t ParserImpl::parse_func_call(const ParsableASTNodePtr &p) {
  p->set_data(at(p->_end_index)->value); /// function name
  ++p->_end_index;

  // No need to check since '(' is what distinguish a function call from an identifier at the first place
  // auto *token = at(p->_end_index); if (token->value != "(") { error("Invalid function call"); }
  ++p->_end_index; /// skip (

  /// args
  while (!eof(p->_end_index) && at(p->_end_index)->value != ")") {
    p->append_child(next_expression(p->_end_index));
    if (at(p->_end_index)->value == ",") { /// skip ,
      ++p->_end_index;
    } else { break; }
  }
  peek(p->_end_index, TokenType::PUNCTUATION, ")");
  ++p->_end_index;

  return p->_end_index;
}
