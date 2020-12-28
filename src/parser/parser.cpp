#include "parser.h"
#include "base.h"
#include "compiler_session.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_control_flow.h"
#include "src/analysis/analysis.h"
#include "src/ast/ast_member_access.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_ty.h"
#include "src/common.h"
#include "intrinsic.h"
#include "token.h"
#include <memory>

using namespace tanlang;
// TODO: move type resolving and other stuff to analysis phase

Parser::Parser(vector<Token *> tokens, const str &filename, CompilerSession *cs)
    : _tokens(std::move(tokens)), _filename(filename), _cs(cs) {}

ASTNodePtr Parser::peek(size_t &index, TokenType type, const str &value) {
  if (index >= _tokens.size()) { report_error(_filename, _tokens.back(), "Unexpected EOF"); }
  Token *token = _tokens[index];
  if (token->type != type || token->value != value) {
    report_error(_filename, token, "Expect '" + value + "', but got '" + token->value + "' instead");
  }
  return peek(index);
}

ASTNodePtr Parser::peek_keyword(Token *token, size_t &index) {
  ASTNodePtr ret = nullptr;
  switch (hashed_string{token->value.c_str()}) {
    case "var"_hs:
      ret = ast_create_var_decl(_cs);
    case "enum"_hs:
      ret = ast_create_enum(_cs);
    case "fn"_hs:
    case "pub"_hs:
    case "extern"_hs:
      ret = ast_create_func_decl(_cs);
    case "import"_hs:
      ret = ast_create_import(_cs);
    case "if"_hs:
      ret = ast_create_if(_cs);
    case "else"_hs:
      ret = ast_create_else(_cs);
    case "return"_hs:
      ret = ast_create_return(_cs);
    case "while"_hs:
    case "for"_hs:
      ret = ast_create_loop(_cs);
    case "struct"_hs:
      ret = ast_create_struct_decl(_cs);
    case "break"_hs:
    case "continue"_hs:
      ret = ast_create_break_or_continue(_cs);
    case "as"_hs:
      ret = ast_create_cast(_cs);
    default:
      return nullptr;
  }
  ret->_token = token;
  ret->_start_index = index;
  return ret;
}

ASTNodePtr Parser::peek(size_t &index) {
  if (index >= _tokens.size()) { return nullptr; }
  Token *token = _tokens[index];
  /// skip comments
  while (token->type == TokenType::COMMENTS) {
    ++index;
    token = _tokens[index];
  }
  ASTNodePtr node;
  if (token->value == "@") { /// intrinsics
    node = std::make_shared<Intrinsic>(token, index);
  } else if (token->value == "=" && token->type == TokenType::BOP) {
    node = ast_create_assignment(_cs);
  } else if (token->value == "!" || token->value == "~") {
    node = ast_create_not(_cs);
  } else if (token->value == "[") {
    auto prev = this->at(index - 1);
    if (prev->type != TokenType::ID && prev->value != "]" && prev->value != ")") {
      /// array literal if there is no identifier, "]", or ")" before
      node = ast_create_array_literal(_cs);
    } else {
      /// otherwise bracket access
      node = ast_create_member_access(_cs);
    }
  } else if (token->type == TokenType::RELOP) { /// comparisons
    node = ast_create_comparison(_cs, token->value);
  } else if (token->type == TokenType::INT || token->type == TokenType::FLOAT) {
    node = ast_create_numeric_literal(_cs);
  } else if (token->type == TokenType::STRING) { /// string literal
    node = ast_create_string_literal(_cs, token->value);
  } else if (token->type == TokenType::CHAR) { /// char literal
    node = ast_create_char_literal(_cs);
  } else if (token->type == TokenType::ID) {
    Token *next = _tokens[index + 1];
    if (next->value == "(") {
      node = ast_create_func_call(_cs);
    } else {
      node = ast_create_identifier(_cs, token->value);
    }
  } else if (token->type == TokenType::PUNCTUATION && token->value == "(") {
    node = ast_create_parenthesis(_cs);
  } else if (token->type == TokenType::KEYWORD) { /// keywords
    node = peek_keyword(token, index);
    if (!node) { report_error(_filename, token, "Keyword not implemented: " + token->to_string()); }
  } else if (token->type == TokenType::BOP && token->value == ".") { /// member access
    node = ast_create_member_access(_cs);
  } else if (check_typename_token(token)) { /// types
    node = ast_create_ty(_cs);
  } else if (token->value == "&") {
    node = ast_create_ampersand(_cs);
  } else if (token->type == TokenType::PUNCTUATION && token->value == "{") { /// statement(s)
    node = ast_create_statement(_cs);
  } else if (token->type == TokenType::BOP && check_arithmetic_token(token)) { /// arithmetic operators
    node = ast_create_arithmetic(_cs, token->value);
  } else if (check_terminal_token(token)) { /// this MUST be the last thing to check
    return nullptr;
  } else {
    report_error(_filename, token, "Unknown token " + token->to_string());
  }
  node->_token = token;
  node->_start_index = node->_end_index = index;
  return node;
}

ASTNodePtr Parser::next_expression(size_t &index, int rbp) {
  ASTNodePtr node = peek(index);
  ++index;
  if (!node) { return nullptr; }
  auto n = node;
  index = parse_node(n);
  auto left = n;
  node = peek(index);
  if (!node) { return left; }
  while (rbp < node->_lbp) {
    node = peek(index);
    n = node;
    index = parse_node(left, n);
    left = n;
    node = peek(index);
    if (!node) { break; }
  }
  return left;
}

/// current token should be "[" when this is called.
size_t Parser::parse_ty_array(const ASTTyPtr &p) {
  ++p->_end_index; /// skip "["
  ASTNodePtr element = nullptr;
  /// element type
  if (at(p->_end_index)->value == "]") { /// empty
    error("The array type and size must be specified");
  } else {
    element = peek(p->_end_index);
    if (element->_type != ASTType::TY) { error("Expect a type"); }
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

size_t Parser::parse_ty_struct(const ASTTyPtr &p) {
  ++p->_end_index; /// skip "struct"
  /// struct typename
  auto id = peek(p->_end_index);
  if (id->_type != ASTType::ID) { error("Expecting a typename"); }
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
    if (!comp_stmt || comp_stmt->_type != ASTType::STATEMENT) { error("Invalid struct body"); }

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
      } else { error("Invalid struct member"); }
      auto name = var_decl->_name;
      p->_member_names.push_back(name);
      p->_member_indices[name] = i;
    }
  } else { p->_is_forward_decl = true; }
  return p->_end_index;
}

size_t Parser::parse_node(const ASTNodePtr &p) {
  p->_end_index = p->_start_index;
  // TODO: update _cs->_current_token

  /// determine p's type depending on whether p is led or nud
  switch (hashed_string{p->_token->value.c_str()}) {
    case "&"_hs:
      p->_type = ASTType::ADDRESS_OF;
      p->_lbp = op_precedence[p->_type];
      break;
    case "!"_hs:
      p->_type = ASTType::LNOT;
      p->_lbp = op_precedence[p->_type];
      break;
    case "~"_hs:
      p->_type = ASTType::BNOT;
      p->_lbp = op_precedence[p->_type];
      break;
    default:
      break;
  }

  switch (p->_type) {
    case ASTType::PROGRAM: {
      while (!eof(p->_end_index)) {
        auto stmt = ast_create_statement(_cs);
        stmt->_token = at(p->_end_index);
        stmt->_start_index = p->_end_index;
        p->_end_index = parse_node(stmt);
        p->_children.push_back(stmt);
      }
      break;
    }
    case ASTType::STATEMENT: {
      if (at(p->_end_index)->value == "{") { /// compound statement
        ++p->_end_index; /// skip "{"
        while (!eof(p->_end_index)) {
          auto node = peek(p->_end_index);
          while (node) { /// stops at a terminal token
            p->_children.push_back(next_expression(p->_end_index, PREC_LOWEST));
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
          p->_children.push_back(next_expression(p->_end_index, PREC_LOWEST));
          node = peek(p->_end_index);
        }
        ++p->_end_index; /// skip ';'
      }
      break;
    }
    case ASTType::PARENTHESIS: {
      ++p->_end_index; /// skip "("
      while (true) {
        auto *t = at(p->_end_index);
        if (!t) {
          error(p->_end_index - 1, "Unexpected EOF");
        } else if (t->type == TokenType::PUNCTUATION && t->value == ")") { /// end at )
          ++p->_end_index;
          break;
        }
        // FIXME: multiple expressions in the parenthesis?
        /// NOTE: parenthesis without child expression inside are illegal (except function call)
        auto n = next_expression(p->_end_index, PREC_LOWEST);
        if (n) {
          p->_children.push_back(n);
        } else {
          error(p->_end_index, "Unexpected " + t->to_string());
        }
      }
      break;
    }
      ////////////////////////// keywords ////////////////////////////////
    case ASTType::IF: {
      auto pif = ast_cast<ASTIf>(p);
      TAN_ASSERT(pif);
      ++pif->_end_index; /// skip "if"
      /// condition
      auto condition = peek(pif->_end_index, TokenType::PUNCTUATION, "(");
      pif->_end_index = parse_node(condition);
      pif->_children.push_back(condition);
      /// if clause
      auto if_clause = peek(pif->_end_index, TokenType::PUNCTUATION, "{");
      pif->_end_index = parse_node(if_clause);
      pif->_children.push_back(if_clause);

      /// else clause, if any
      auto *token = at(pif->_end_index);
      if (token->type == TokenType::KEYWORD && token->value == "else") {
        auto else_clause = peek(pif->_end_index);
        pif->_end_index = parse_node(else_clause);
        pif->_children.push_back(else_clause);
        pif->_has_else = true;
      }
      break;
    }
      ////////////////////////// prefix ////////////////////////////////
    case ASTType::ADDRESS_OF:
    case ASTType::LNOT:
    case ASTType::BNOT:
    case ASTType::RET: {
      ++p->_end_index;
      p->_children.push_back(next_expression(p->_end_index));
      break;
    }
    case ASTType::SUM: /// unary +
    case ASTType::SUBTRACT: { /// unary -
      ++p->_end_index; /// skip "-" or "+"
      /// higher precedence than infix plus/minus
      p->_lbp = PREC_UNARY;
      auto rhs = next_expression(p->_end_index, p->_lbp);
      if (!rhs) { error(p->_end_index, "Invalid operand"); }
      p->_children.push_back(rhs);
      break;
    }
      ////////////////////////// others /////////////////////////////////
    case ASTType::FUNC_CALL: {
      /// function name
      p->_name = at(p->_end_index)->value;
      ++p->_end_index;

      // No need to check since '(' is what distinguish a function call from an identifier at the first place
      // auto *token = at(p->_end_index); if (token->value != "(") { error("Invalid function call"); }
      ++p->_end_index; /// skip (

      p->_children.push_back(nullptr); /// pointer to ASTFunction (callee), set in analysis phase
      /// args
      while (!eof(p->_end_index) && at(p->_end_index)->value != ")") {
        p->_children.push_back(next_expression(p->_end_index));
        if (at(p->_end_index)->value == ",") { /// skip ,
          ++p->_end_index;
        } else { break; }
      }
      peek(p->_end_index, TokenType::PUNCTUATION, ")");
      ++p->_end_index;
      break;
    }
    case ASTType::VAR_DECL:
      ++p->_end_index; /// skip 'var'
      // fallthrough
    case ASTType::ARG_DECL: {
      /// var name
      auto name_token = at(p->_end_index);
      p->_name = name_token->value;
      ++p->_end_index;
      if (at(p->_end_index)->value == ":") {
        ++p->_end_index;
        /// type
        auto ty = ast_create_ty(_cs);
        ty->_token = at(p->_end_index);
        ty->_end_index = ty->_start_index = p->_end_index;
        ty->_is_lvalue = true;
        p->_end_index = parse_node(ty);
        p->_ty = ty;
      } else { p->_ty = nullptr; }
    }
    case ASTType::ARRAY_LITERAL: {
      ++p->_end_index; /// skip '['
      if (at(p->_end_index)->value == "]") { error(p->_end_index, "Empty array"); }
      auto element_type = ASTType::INVALID;
      while (!eof(p->_end_index)) {
        if (at(p->_end_index)->value == ",") {
          ++p->_end_index;
          continue;
        } else if (at(p->_end_index)->value == "]") {
          ++p->_end_index;
          break;
        }
        auto node = peek(p->_end_index);
        if (!node) { error(p->_end_index, "Unexpected token"); }
        /// check whether element types are the same
        if (element_type == ASTType::INVALID) { element_type = node->_type; }
        else {
          if (element_type != node->_type) {
            error(p->_end_index, "All elements in an array must have the same type");
          }
        }
        if (is_ast_type_in(node->_type, TypeSystem::LiteralTypes)) {
          if (node->_type == ASTType::ARRAY_LITERAL) { ++p->_end_index; }
          p->_end_index = parse_node(node);
          p->_children.push_back(node);
        } else { error(p->_end_index, "Expect literals"); }
      }
      break;
    }
    case ASTType::FUNC_DECL: {
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
      auto id = peek(p->_end_index);
      if (id->_type != ASTType::ID) { error(p->_end_index, "Invalid function name"); }
      p->_end_index = parse_node(id);
      p->_name = id->_name;

      /// arguments
      peek(p->_end_index, TokenType::PUNCTUATION, "(");
      ++p->_end_index;
      if (at(p->_end_index)->value != ")") {
        while (!eof(p->_end_index)) {
          auto arg = ast_create_arg_decl(_cs);
          arg->_token = at(p->_end_index);
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
      ret_ty->_token = at(p->_end_index);
      ret_ty->_end_index = ret_ty->_start_index = p->_end_index;
      p->_end_index = parse_node(ret_ty); /// return type
      p->_children[0] = ret_ty;

      /// body
      if (!p->_is_external) {
        auto body = peek(p->_end_index, TokenType::PUNCTUATION, "{");
        p->_end_index = parse_node(body);
        p->_children.push_back(body);
        _cs->pop_scope();
      }
      break;
    }
    case ASTType::TY: {
      Token *token;
      ASTTyPtr pt = ast_cast<ASTTy>(p);
      TAN_ASSERT(pt);
      while (!eof(p->_end_index)) {
        token = at(p->_end_index);
        if (basic_tys.find(token->value) != basic_tys.end()) { /// base types
          pt->_tyty = TY_OR(pt->_tyty, basic_tys[token->value]);
        } else if (qualifier_tys.find(token->value) != qualifier_tys.end()) { /// TODO: qualifiers
          if (token->value == "*") { /// pointer
            /// swap self and child
            auto sub = std::make_shared<ASTTy>(*pt);
            pt->_tyty = Ty::POINTER;
            pt->_children.clear(); /// clear but memory stays
            pt->_children.push_back(sub);
          }
        } else if (token->type == TokenType::ID) { /// struct or enum
          // TODO: identify type aliases
          pt->_type_name = token->value;
          auto ty = ast_cast<ASTTy>(_cs->get(pt->_type_name));
          if (ty) { *pt = *ty; }
          else { error("Invalid type name"); }
        } else if (token->value == "[") {
          pt->_tyty = Ty::ARRAY;
          pt->_end_index = parse_ty_array(pt);
          break;
        } else if (token->value == "struct") {
          pt->_tyty = Ty::STRUCT;
          pt->_end_index = parse_ty_struct(pt);
          break;
        } else { break; }
        ++pt->_end_index;
      }
      break;
    }
      /////////////////////////////// trivially parsed ASTs ///////////////////////////////////
    case ASTType::ID:
    case ASTType::NUM_LITERAL:
    case ASTType::CHAR_LITERAL:
    case ASTType::STRING_LITERAL:
      ++p->_end_index;
      break;
    default:
      break;
  }
  return p->_end_index;
}

size_t Parser::parse_node(const ASTNodePtr &left, const ASTNodePtr &p) {
  p->_end_index = p->_start_index;
  // TODO: update _cs->_current_token

  /// determine p's type depending on whether p is led or nud
  switch (hashed_string{p->_token->value.c_str()}) {
    case "&"_hs:
      p->_type = ASTType::BAND;
      p->_lbp = op_precedence[p->_type];
      break;
    default:
      break;
  }

  switch (p->_type) {
    case ASTType::MEMBER_ACCESS: {
      auto pma = ast_cast<ASTMemberAccess>(p);
      TAN_ASSERT(pma);
      if (at(p->_end_index)->value == "[") { pma->_access_type = MemberAccessType::MemberAccessBracket; }

      ++p->_end_index; /// skip "." or "["
      p->_children.push_back(left); /// lhs
      auto right = peek(p->_end_index);
      if (pma->_access_type != MemberAccessType::MemberAccessBracket && right->_token->value == "*") {
        /// pointer dereference
        pma->_access_type = MemberAccessType::MemberAccessDeref;
        ++p->_end_index;
      } else {
        p->_end_index = parse_node(right);
        p->_children.push_back(right);
      }
      // TODO: member function call, (insert 'self' as the first arg)
      // if (right->_type != ASTType::FUNC_CALL)

      if (pma->_access_type == MemberAccessType::MemberAccessBracket) {
        auto rhs = p->_children[1];
        ++p->_end_index; /// skip "]" if this is a bracket access
        ASTTyPtr ty = left->_ty;
        TAN_ASSERT(ty->_is_ptr);
        ty = std::make_shared<ASTTy>(*get_contained_ty(_cs, ty));
        ty->_is_lvalue = true;
        if (!ty) { error("Unable to perform bracket access"); }
        p->_ty = ty;
        // TODO
        //  if (rhs->_type == ASTType::NUM_LITERAL) {
        //    if (!rhs->_ty->_is_int) { error("Expect an integer specifying array size"); }
        //    if (left->_ty->_is_array
        //        && (size_t) /* underflow */ std::get<uint64_t>(rhs->_value) >= left->_ty->get_n_elements()) {
        //      error("Index " + std::to_string(size->_ivalue) + " out of bound, the array size is "
        //          + std::to_string(left->get_ty()->get_n_elements()));
        //    }
        //  }
      } else if (pma->_access_type == MemberAccessType::MemberAccessDeref) { /// pointer dereference
        resolve_ptr_deref(left);
      } else if (p->_children[1]->_type == ASTType::ID) { /// member variable or enum
        if (left->_ty->_is_enum) {
          pma->_access_type = MemberAccessType::MemberAccessEnumValue;
          auto enum_ = ast_cast<ASTEnum>(left->_ty);
          p->_ty = enum_;
          _enum_value = enum_->get_enum_value(p->_children[1]->_name);
        } else {
          pma->_access_type = MemberAccessType::MemberAccessMemberVariable;
          if (!left->_ty->_is_lvalue && !left->_ty->_is_ptr) { error("Invalid left-hand operand"); }
          auto rhs = p->_children[1];
          str m_name = rhs->_name;
          std::shared_ptr<ASTTy> struct_ast = nullptr;
          /// auto dereference pointers
          if (left->_ty->_is_ptr) {
            struct_ast = ast_cast<ASTTy>(_cs->get(left->_ty->get_contained_ty()->get_type_name()));
          } else {
            struct_ast = ast_cast<ASTTy>(_cs->get(left->_ty->_type_name));
          }
          pma->_access_idx = struct_ast->get_member_index(m_name);
          auto member = struct_ast->get_member(pma->_access_idx);
          p->_ty = std::make_shared<ASTTy>(*member->get_ty());
          p->_ty->_is_lvalue = true;
        }
      } else if (p->_children[1]->_type == ASTType::FUNC_CALL) { /// method call
        /// TODO: make `self` reference instead of pointer
        auto func = ast_cast<ASTFunctionCall>(p->_children[1]);
        TAN_ASSERT(func);
        func->_do_resolve = false;
        p->_end_index = parse_node(_cs, func);
        if (!left->_ty->_is_lvalue && !left->_ty->_is_ptr) {
          error("Method calls require left-hand operand to be an lvalue or a pointer");
        }
        /// auto dereference pointers
        if (left->_ty->_is_lvalue && !left->_ty->_is_ptr) {
          func->_children.insert(func->_children.begin(), ASTAmpersand::CreateAddressOf(left));
        } else {
          func->_children.insert(func->_children.begin(), left);
        }
        func->resolve();
        p->_ty = func->_ty;
        pma->_access_type = MemberAccessType::MemberAccessMemberFunction;
      } else { error("Invalid right-hand operand"); }
      break;
    }
    case ASTType::BAND:
    case ASTType::CAST:
    case ASTType::ASSIGN:
    case ASTType::GT:
    case ASTType::GE:
    case ASTType::LT:
    case ASTType::LE:
    case ASTType::EQ:
    case ASTType::NE:
    case ASTType::SUM:
    case ASTType::SUBTRACT:
    case ASTType::MULTIPLY:
    case ASTType::DIVIDE:
    case ASTType::MOD: {
      ++p->_end_index; /// skip operator
      p->_children.push_back(left); /// lhs
      auto n = next_expression(p->_end_index, p->_lbp);
      if (!n) { error(p->_end_index, "Invalid operand"); }
      p->_children.push_back(n);
      break;
    }
    default:
      break;
  }
  return p->_end_index;
}

ASTNodePtr Parser::parse() {
  _root = ast_create_program(_cs);
  parse_node(_root);
  return _root;
}

Token *Parser::at(const size_t idx) const {
  if (this->eof(idx)) { report_error(_filename, _tokens.back(), "Unexpected EOF"); }
  return _tokens[idx];
}

str Parser::get_filename() const { return _filename; }

bool Parser::eof(size_t index) const { return index >= _tokens.size(); }

void Parser::error(const str &error_message) {
  if (_cs && _cs->_current_token) {
    report_error(get_filename(), _cs->_current_token, error_message);
  } else { report_error(error_message); }
}

void Parser::error(size_t i, const str &error_message) { report_error(get_filename(), at(i), error_message); }

