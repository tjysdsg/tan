#include "parser.h"
#include "base.h"
#include "compiler_session.h"
#include "src/parser/parser_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "src/common.h"
#include "src/ast/ast_node.h"
#include "token.h"

using namespace tanlang;

// TODO: double-check

/// current token should be "[" when this is called.
size_t ParserImpl::parse_ty_array(const ASTTyPtr &p) {
  ++p->_end_index; /// skip "["
  ASTNodePtr element = nullptr;
  /// element type
  if (at(p->_end_index)->value == "]") { /// empty
    error(p->_end_index, "The array type and size must be specified");
  } else {
    element = peek(p->_end_index);
    if (element->get_node_type() != ASTType::TY) {
      error(p->_end_index, "Expect a type");
    }
    p->_end_index = parse_node(element);
  }
  peek(p->_end_index, TokenType::PUNCTUATION, ",");
  ++p->_end_index; /// skip ","

  /// size
  ASTTyPtr ety = ast_cast<ASTTy>(element);
  TAN_ASSERT(ety);
  ASTNodePtr size = peek(p->_end_index);
  if (size->get_node_type() != ASTType::NUM_LITERAL) { error(p->_end_index, "Expect an unsigned integer"); }
  p->_end_index = parse_node(size);
  if (size->_ty->_is_float || static_cast<int64_t>(std::get<uint64_t>(size->_value)) < 0) {
    error(p->_end_index, "Expect an unsigned integer");
  }
  p->_array_size = std::get<uint64_t>(size->_value);
  p->append_child(ety);
  /// set _type_name to '[<element type>, <n_elements>]'
  p->_type_name = "[" + p->_type_name + ", " + std::to_string(p->_array_size) + "]";
  ++p->_end_index; /// skip "]"
  return p->_end_index;
}

size_t ParserImpl::parse_ty_struct(const ASTTyPtr &p) {
  ++p->_end_index; /// skip "struct"
  /// struct typename
  auto id = peek(p->_end_index);
  if (id->get_node_type() != ASTType::ID) {
    error(p->_end_index, "Expecting a typename");
  }
  p->_type_name = id->_name;

  auto forward_decl = _cs->get(p->_type_name);
  if (!forward_decl) {
    _cs->add(p->_type_name, p); /// add type to current scope
  } else {
    /// replace forward decl with p (even if p is a forward declaration too)
    _cs->set(p->_type_name, p);
  }

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
        var_decl = member;
        p->append_child(var_decl->_ty);
      } else if (member->get_node_type() == ASTType::ASSIGN) { /// member variable with an initial value
        var_decl = member->get_child_at(0);
        ASTNodePtr initial_value = member->get_child_at(1);
        // TODO: check if value is compile-time known
        p->append_child(initial_value->_ty); /// initial value is set to ASTTy in ASTLiteral::get_ty()
      } else {
        error(member->_end_index, "Invalid struct member");
      }
      auto name = var_decl->_name;
      p->_member_names.push_back(name);
      p->_member_indices[name] = i;
    }
  } else {
    p->_is_forward_decl = true;
  }
  return p->_end_index;
}

size_t ParserImpl::parse_ty(ParsableASTNodePtr &p) {
  ASTTyPtr pty = ast_cast<ASTTy>(p);
  TAN_ASSERT(pty);

  Token *token;
  while (!eof(p->_end_index)) {
    token = at(p->_end_index);
    auto qb = ASTTy::basic_tys.find(token->value);
    auto qq = ASTTy::qualifier_tys.find(token->value);
    if (qb != ASTTy::basic_tys.end()) { /// base types
      pty->_tyty = TY_OR(pty->_tyty, qb->second);
    } else if (qq != ASTTy::qualifier_tys.end()) { /// TODO: qualifiers
      if (token->value == "*") { /// pointer
        auto sub = std::make_shared<ASTTy>(*p);
        // TODO: use factory to create pointer ty
        pty->_tyty = Ty::POINTER;
        pty->clear_children();
        pty->append_child(sub);
      }
    } else if (token->type == TokenType::ID) { /// struct or enum
      // TODO: identify type aliases
      *p = *(_cs->get_type(token->value));
      if (!p) {
        error(p->_end_index, "Invalid type name");
      }
    } else if (token->value == "[") {
      pty->_tyty = Ty::ARRAY;
      p->_end_index = parse_ty_array(p);
      break;
    } else if (token->value == "struct") {
      pty->_tyty = Ty::STRUCT;
      p->_end_index = parse_ty_struct(p);
      break;
    } else {
      break;
    }
    ++p->_end_index;
  }
  return p->_end_index;
}
