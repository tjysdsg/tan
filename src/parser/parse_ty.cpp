#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_ty.h"
#include "src/ast/ast_node.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/factory.h"
#include "compiler_session.h"

using namespace tanlang;

/// current token should be "[" when this is called
size_t ParserImpl::parse_ty_array(const ASTTyPtr &p) {
  ++p->_end_index; /// skip "["
  ParsableASTNodePtr element = nullptr;
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

  ParsableASTNodePtr size = peek(p->_end_index);
  if (size->get_node_type() != ASTType::NUM_LITERAL) { error(p->_end_index, "Expect an unsigned integer"); }
  p->_end_index = parse_node(size);

  TAN_ASSERT(ast_cast<ASTNode>(size));
  if (ast_cast<ASTNode>(size)->_ty->_is_float || static_cast<int64_t>(size->get_data<uint64_t>()) < 0) {
    error(p->_end_index, "Expect an unsigned integer");
  }

  p->_array_size = size->get_data<uint64_t>();
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
        p->_tyty = Ty::POINTER;
        p->clear_children();
        p->append_child(sub);
      }
    } else if (token->type == TokenType::ID) { /// struct or enum
      // TODO: identify type aliases
      *p = *(_cs->get_type(token->value));
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
