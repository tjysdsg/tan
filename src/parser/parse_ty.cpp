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
    if (element->_type != ASTType::TY) {
      error(p->_end_index, "Expect a type");
    }
    p->_end_index = parse_node(element);
  }
  peek(p->_end_index, TokenType::PUNCTUATION, ",");
  ++p->_end_index; /// skip ","

  /// size
  ASTTyPtr ety = ast_cast<ASTTy>(element);
  TAN_ASSERT(ety);
  auto size = peek(p->_end_index);
  if (size->_type != ASTType::NUM_LITERAL) { error(p->_end_index, "Expect an unsigned integer"); }
  p->_end_index = parse_node(size);
  if (size->_ty->_is_float || static_cast<int64_t>(std::get<uint64_t>(size->_value)) < 0) {
    error(p->_end_index, "Expect an unsigned integer");
  }
  p->_array_size = std::get<uint64_t>(size->_value);
  p->_children.push_back(ety);
  /// set _type_name to '[<element type>, <n_elements>]'
  p->_type_name = "[" + p->_type_name + ", " + std::to_string(p->_array_size) + "]";
  ++p->_end_index; /// skip "]"
  return p->_end_index;
}

size_t ParserImpl::parse_ty_struct(const ASTTyPtr &p) {
  ++p->_end_index; /// skip "struct"
  /// struct typename
  auto id = peek(p->_end_index);
  if (id->_type != ASTType::ID) {
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
    if (!comp_stmt || comp_stmt->_type != ASTType::STATEMENT) {
      error(comp_stmt->_end_index, "Invalid struct body");
    }

    /// resolve_ty member names and types
    auto members = comp_stmt->_children;
    ASTNodePtr var_decl = nullptr;
    size_t n = comp_stmt->_children.size();
    p->_member_names.reserve(n);
    p->_children.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      if (members[i]->_type == ASTType::VAR_DECL) { /// member variable without initial value
        var_decl = members[i];
        p->_children.push_back(var_decl->_ty);
      } else if (members[i]->_type == ASTType::ASSIGN) { /// member variable with an initial value
        var_decl = members[i]->_children[0];
        auto initial_value = members[i]->_children[1];
        // TODO: check if value is compile-time known
        p->_children.push_back(initial_value->_ty); /// initial value is set to ASTTy in ASTLiteral::get_ty()
      } else {
        error(members[i]->_end_index, "Invalid struct member");
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

size_t ParserImpl::parse_ty(ASTTyPtr &p) {
  Token *token;
  while (!eof(p->_end_index)) {
    token = at(p->_end_index);
    auto qb = ASTTy::basic_tys.find(token->value);
    auto qq = ASTTy::qualifier_tys.find(token->value);
    if (qb != ASTTy::basic_tys.end()) { /// base types
      p->_tyty = TY_OR(p->_tyty, qb->second);
    } else if (qq != ASTTy::qualifier_tys.end()) { /// TODO: qualifiers
      if (token->value == "*") { /// pointer
        auto sub = std::make_shared<ASTTy>(*p);
        // TODO: use factory to create pointer ty
        p->_tyty = Ty::POINTER;
        p->_children.clear();
        p->_children.push_back(sub);
      }
    } else if (token->type == TokenType::ID) { /// struct or enum
      // TODO: identify type aliases
      p = _cs->get_type(token->value);
      if (!p) {
        error(p->_end_index, "Invalid type name");
      }
    } else if (token->value == "[") {
      p->_tyty = Ty::ARRAY;
      p->_end_index = parse_ty_array(p);
      break;
    } else if (token->value == "struct") {
      p->_tyty = Ty::STRUCT;
      p->_end_index = parse_ty_struct(p);
      break;
    } else {
      break;
    }
    ++p->_end_index;
  }
  return p->_end_index;
}
