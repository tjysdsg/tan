#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_ty.h"
#include "src/ast/ast_node.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/factory.h"
#include "compiler_session.h"
#include <fmt/core.h>

using namespace tanlang;

size_t ParserImpl::parse_ty_array(const ASTTyPtr &p) {
  bool done = false;
  while (!done) {
    /// current token should be "[" right now
    ++p->_end_index; /// skip "["

    /// subtype
    ASTTyPtr sub = make_ptr<ASTTy>(*p);
    p->_tyty = Ty::ARRAY;
    p->clear_children();
    p->append_child(sub);

    /// size
    ParsableASTNodePtr _size = peek(p->_end_index);
    if (_size->get_node_type() != ASTType::NUM_LITERAL) {
      error(p->_end_index, "Expect an unsigned integer as the array size");
    }
    p->_end_index = parse_node(_size);

    ASTNodePtr size = ast_must_cast<ASTNode>(_size);
    if (size->_ty->_is_float || static_cast<int64_t>(size->get_data<uint64_t>()) < 0) {
      error(p->_end_index, "Expect an unsigned integer as the array size");
    }

    p->_array_size = size->get_data<uint64_t>();

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

size_t ParserImpl::parse_ty_struct(const ASTTyPtr &p) {
  ++p->_end_index; /// skip "struct"
  /// struct typename
  auto id = peek(p->_end_index);
  if (id->get_node_type() != ASTType::ID) {
    error(p->_end_index, "Expecting a typename");
  }
  p->_type_name = id->get_data<str>();

  /// struct body
  if (at(p->_end_index)->value == "{") {
    auto comp_stmt = next_expression(p->_end_index);
    if (!comp_stmt || comp_stmt->get_node_type() != ASTType::STATEMENT) {
      error(comp_stmt->_end_index, "Invalid struct body");
    }

    /// resolve_ty member names and types
    ASTNodePtr var_decl = nullptr;
    size_t n = comp_stmt->get_children_size();
    p->_member_names.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      auto member = comp_stmt->get_child_at(i);
      if (member->get_node_type() == ASTType::VAR_DECL) { /// member variable without initial value
        var_decl = ast_cast<ASTNode>(member);
        p->append_child(var_decl->_ty);
      } else if (member->get_node_type() == ASTType::ASSIGN) { /// member variable with an initial value
        var_decl = member->get_child_at<ASTNode>(0);
        ASTNodePtr initial_value = member->get_child_at<ASTNode>(1);
        // TODO: check if value is compile-time known
        p->append_child(initial_value->_ty); /// initial value is set to ASTTy in ASTLiteral::get_ty()
      } else {
        error(member->_end_index, "Invalid struct member");
      }
      auto name = var_decl->get_data<str>();
      p->_member_names.push_back(name);
      p->_member_indices[name] = i;
    }
  } else {
    p->_is_forward_decl = true;
  }
  return p->_end_index;
}

size_t ParserImpl::parse_ty(const ASTTyPtr &p) {
  while (!eof(p->_end_index)) {
    Token *token = at(p->_end_index);
    auto qb = ASTTy::basic_tys.find(token->value);
    auto qq = ASTTy::qualifier_tys.find(token->value);

    if (qb != ASTTy::basic_tys.end()) { /// base types
      p->_tyty = TY_OR(p->_tyty, qb->second);
    } else if (qq != ASTTy::qualifier_tys.end()) { /// TODO: qualifiers
      if (token->value == "*") { /// pointer
        auto sub = std::make_shared<ASTTy>(*p);
        p->_tyty = Ty::POINTER;
        p->clear_children();
        p->append_child(sub);
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
  } else if (token->value == "struct") { /// struct declaration, TODO: make a parse_decl function
    p->_tyty = Ty::STRUCT;
    p->_end_index = parse_ty_struct(p);
  }
  return p->_end_index;
}
