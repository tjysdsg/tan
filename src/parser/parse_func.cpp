#include "parser.h"
#include "base.h"
#include "compiler_session.h"
#include "src/parser/parser_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_control_flow.h"
#include "src/ast/ast_member_access.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "src/common.h"
#include "intrinsic.h"
#include "token.h"
#include <memory>
#include <utility>

using namespace tanlang;

// TODO: move scope checking to analysis phase

size_t ParserImpl::parse_func_decl(const ASTNodePtr &p) {
  if (at(p->_start_index)->value == "fn") {
    /// skip "fn"
    ++p->_end_index;
  } else if (at(p->_start_index)->value == "pub") {
    p->_is_public = true;
    /// skip "pub fn"
    p->_end_index = p->_start_index + 2;
  } else if (at(p->_start_index)->value == "extern") {
    p->_is_external = true;
    /// skip "pub fn"
    p->_end_index = p->_start_index + 2;
  } else { TAN_ASSERT(false); }

  /// function return type, set later
  p->_children.push_back(nullptr);

  /// function name
  // Don't use peek since it look ahead and returns ASTType::FUNCTION when it finds "(",
  // but we only want the function name as an identifier
  // auto id = peek(p->_end_index);
  Token *id_token = at(p->_end_index);
  auto id = ast_create_identifier(_cs, id_token->value);
  id->_start_index = id->_end_index = p->_end_index;

  if (id->_type != ASTType::ID) { error(p->_end_index, "Invalid function name"); }
  p->_end_index = parse_node(id);
  p->_name = id->_name;

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
      p->_children.push_back(arg);
      if (at(p->_end_index)->value == ",") {
        ++p->_end_index;
      } else { break; }
    }
  }
  peek(p->_end_index, TokenType::PUNCTUATION, ")");
  ++p->_end_index;
  peek(p->_end_index, TokenType::PUNCTUATION, ":");
  ++p->_end_index;
  auto ret_ty = ast_create_ty(_cs);
  ret_ty->set_token(at(p->_end_index));
  ret_ty->_end_index = ret_ty->_start_index = p->_end_index;
  p->_end_index = parse_node(ret_ty); /// return type
  p->_children[0] = ret_ty;

  /// body
  if (!p->_is_external) {
    auto body = peek(p->_end_index, TokenType::PUNCTUATION, "{");
    p->_end_index = parse_node(body);
    p->_children.push_back(body);
  }
  _cs->pop_scope(); /// pop function scope

  return p->_end_index;
}

size_t ParserImpl::parse_func_call(const ASTNodePtr &p) {
  /// function name
  p->_name = at(p->_end_index)->value;
  ++p->_end_index;

  // No need to check since '(' is what distinguish a function call from an identifier at the first place
  // auto *token = at(p->_end_index); if (token->value != "(") { error("Invalid function call"); }
  ++p->_end_index; /// skip (

  /// args
  while (!eof(p->_end_index) && at(p->_end_index)->value != ")") {
    p->_children.push_back(next_expression(p->_end_index));
    if (at(p->_end_index)->value == ",") { /// skip ,
      ++p->_end_index;
    } else { break; }
  }
  peek(p->_end_index, TokenType::PUNCTUATION, ")");
  ++p->_end_index;

  return p->_end_index;
}
