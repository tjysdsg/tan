#include "base.h"
#include "compiler_session.h"
#include "src/parser/parser_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/stmt.h"
#include "src/ast/expr.h"
#include "src/ast/decl.h"
#include "src/ast/ast_control_flow.h"
#include "src/ast/ast_member_access.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_type.h"
#include "src/common.h"
#include "intrinsic.h"
#include "token.h"
#include <memory>
#include <utility>

using namespace tanlang;
using tanlang::TokenType; // distinguish from the one in winnt.h

// TODO: move type resolving to analysis phase

ParserImpl::ParserImpl(vector<Token *> tokens, str filename, CompilerSession *cs)
    : _tokens(std::move(tokens)), _filename(std::move(filename)), _cs(cs) {}

ASTBasePtr ParserImpl::peek(size_t &index, TokenType type, const str &value) {
  if (index >= _tokens.size()) { report_error(_filename, _tokens.back(), "Unexpected EOF"); }
  Token *token = _tokens[index];
  if (token->type != type || token->value != value) {
    report_error(_filename, token, "Expect '" + value + "', but got '" + token->value + "' instead");
  }
  return peek(index);
}

ASTBasePtr ParserImpl::peek_keyword(Token *token, size_t &index) {
  ASTBasePtr ret = nullptr;
  switch (hashed_string{token->value.c_str()}) {
    case "var"_hs:
      ret = VarDecl::Create();
      break;
    case "enum"_hs:
      // TODO: implement enum
      TAN_ASSERT(false);
      break;
    case "fn"_hs:
    case "pub"_hs:
    case "extern"_hs:
      ret = FunctionDecl::Create();
      break;
    case "import"_hs:
      ret = ast_create_import(_cs);
      break;
    case "if"_hs:
      ret = ast_create_if(_cs);
      break;
    case "else"_hs:
      ret = ast_create_else(_cs);
      break;
    case "return"_hs:
      ret = ast_create_return(_cs);
      break;
    case "while"_hs:
    case "for"_hs:
      ret = ast_create_loop(_cs);
      break;
    case "struct"_hs:
      ret = StructDecl::Create();
      break;
    case "break"_hs:
      ret = ast_create_break(_cs);
      break;
    case "continue"_hs:
      ret = ast_create_continue(_cs);
      break;
    case "as"_hs:
      ret = ast_create_cast(_cs);
      break;
    default:
      return nullptr;
  }
  ret->set_token(token);
  ret->_start_index = index;
  return ret;
}

ASTBasePtr ParserImpl::peek(size_t &index) {
  if (index >= _tokens.size()) { return nullptr; }
  Token *token = _tokens[index];
  /// skip comments
  while (token && token->type == TokenType::COMMENTS) {
    ++index;
    token = _tokens[index];
  }
  // check if there are tokens after the comment
  if (index >= _tokens.size()) { return nullptr; }

  ASTBasePtr node = nullptr;
  if (token->value == "@") { /// intrinsics
    node = ast_create_intrinsic(_cs);
  } else if (token->value == "=" && token->type == TokenType::BOP) {
    node = BinaryOperator::Create(BinaryOpKind::ASSIGN);
  } else if (token->value == "!") { /// logical not
    node = UnaryOperator::Create(UnaryOpKind::LNOT);
  } else if (token->value == "~") { /// binary not
    node = UnaryOperator::Create(UnaryOpKind::BNOT);
  } else if (token->value == "[") {
    auto prev = this->at(index - 1);
    if (prev->type != TokenType::ID && prev->value != "]" && prev->value != ")") {
      /// array literal if there is no identifier, "]", or ")" before
      node = ArrayLiteral::Create();
    } else {
      /// otherwise bracket access
      node = ast_create_member_access(_cs);
    }
  } else if (token->type == TokenType::RELOP) { /// comparisons
    BinaryOpKind op = BinaryOpKind::INVALID;
    switch (hashed_string{token->value.c_str()}) {
      case ">"_hs:
        op = BinaryOpKind::GT;
        break;
      case ">="_hs:
        op = BinaryOpKind::GE;
        break;
      case "<"_hs:
        op = BinaryOpKind::LT;
        break;
      case "<="_hs:
        op = BinaryOpKind::LE;
        break;
      case "=="_hs:
        op = BinaryOpKind::EQ;
        break;
      case "!="_hs:
        op = BinaryOpKind::NE;
        break;
      default:
        return nullptr;
    }
    node = BinaryOperator::Create(op);
  } else if (token->type == TokenType::INT) {
    node = IntegerLiteral::Create((uint64_t) std::stol(token->value), token->is_unsigned);
  } else if (token->type == TokenType::FLOAT) {
    node = FloatLiteral::Create(std::stod(token->value));
  } else if (token->type == TokenType::STRING) { /// string literal
    node = StringLiteral::Create(token->value);
  } else if (token->type == TokenType::CHAR) { /// char literal
    node = CharLiteral::Create(static_cast<uint8_t>(token->value[0]));
  } else if (check_typename_token(token)) { /// types, must be before ID
    node = ASTType::Create();
  } else if (token->type == TokenType::ID) {
    Token *next = _tokens[index + 1];
    if (next->value == "(") {
      node = ast_create_func_call(_cs);
    } else {
      node = Identifier::Create(token->value);
    }
  } else if (token->type == TokenType::PUNCTUATION && token->value == "(") {
    node = ast_create_parenthesis(_cs);
  } else if (token->type == TokenType::KEYWORD) { /// keywords
    node = peek_keyword(token, index);
    if (!node) { report_error(_filename, token, "Keyword not implemented: " + token->to_string()); }
  } else if (token->type == TokenType::BOP && token->value == ".") { /// member access
    node = ast_create_member_access(_cs);
  } else if (token->value == "&") {
    // TODO: handle ambiguous node
    node = ast_create_ampersand(_cs);
  } else if (token->type == TokenType::PUNCTUATION && token->value == "{") { /// statement(s)
    node = Stmt::Create();
  } else if (token->type == TokenType::BOP && check_arithmetic_token(token)) { /// arithmetic operators
    // FIXME: BOP or UOP? ambiguous
    BinaryOpKind op = BinaryOpKind::INVALID;
    switch (hashed_string{token->value.c_str()}) {
      case "+"_hs:
        op = BinaryOpKind::SUM;
        break;
      case "-"_hs:
        op = BinaryOpKind::SUBTRACT;
        break;
      case "*"_hs:
        op = BinaryOpKind::MULTIPLY;
        break;
      case "/"_hs:
        op = BinaryOpKind::DIVIDE;
        break;
      case "%"_hs:
        op = BinaryOpKind::MOD;
        break;
      default:
        return nullptr;
    }
    node = BinaryOperator::Create(op);
  } else if (check_terminal_token(token)) { /// this MUST be the last thing to check
    return nullptr;
  } else {
    report_error(_filename, token, "Unknown token " + token->to_string());
  }
  node->set_token(token);
  node->_start_index = node->_end_index = index;
  return node;
}

ASTBasePtr ParserImpl::next_expression(size_t &index, int rbp) {
  ASTBasePtr node = peek(index);
  ++index;
  if (!node) { return nullptr; }
  auto n = node;
  index = parse_node(n);
  auto left = n;
  node = peek(index);
  if (!node) { return left; }
  while (rbp < node->get_lbp()) {
    node = peek(index);
    n = node;
    index = parse_node(left, n);
    left = n;
    node = peek(index);
    if (!node) { break; }
  }
  return left;
}

size_t ParserImpl::parse_node(const ASTBasePtr &p) {
  p->_end_index = p->_start_index;

  // FIXME: special tokens that require whether p is led or nud to determine the node type
  if (p->get_token() != nullptr) {
    switch (hashed_string{p->get_token_str().c_str()}) {
      case "&"_hs:
        p->set_node_type(ASTNodeType::ADDRESS_OF);
        p->set_lbp(ASTBase::OpPrecedence[p->get_node_type()]);
        break;
      default:
        break;
    }
  }

  switch (p->get_node_type()) {
    case ASTNodeType::PROGRAM:
      parse_program(p);
      break;
    case ASTNodeType::STATEMENT:
      parse_stmt(p);
      break;
    case ASTNodeType::PARENTHESIS: {
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
          p->append_child(n);
        } else {
          error(p->_end_index, "Unexpected " + t->to_string());
        }
      }
      break;
    }
    case ASTNodeType::IMPORT:
      parse_import(p);
      break;
    case ASTNodeType::INTRINSIC:
      parse_intrinsic(p);
      break;
      ////////////////////////// control flow ////////////////////////////////
    case ASTNodeType::IF:
      parse_if(p);
      break;
    case ASTNodeType::ELSE:
      parse_else(p);
      break;
    case ASTNodeType::LOOP:
      parse_loop(p);
      break;
      ////////////////////////// prefix ////////////////////////////////
    case ASTNodeType::ADDRESS_OF:
    case ASTNodeType::UOP:
      parse_uop(p);
      break;
    case ASTNodeType::RET: {
      ++p->_end_index;
      p->append_child(next_expression(p->_end_index));
      break;
    }
    case ASTNodeType::SUM: /// unary +
    case ASTNodeType::SUBTRACT: { /// unary -
      ++p->_end_index; /// skip "-" or "+"
      /// higher precedence than infix plus/minus
      p->set_lbp(PREC_UNARY);
      auto rhs = next_expression(p->_end_index, p->get_lbp());
      if (!rhs) { error(p->_end_index, "Invalid operand"); }
      p->append_child(rhs);
      break;
    }
      ////////////////////////// others /////////////////////////////////
    case ASTNodeType::FUNC_CALL:
      parse_func_call(p);
      break;
    case ASTNodeType::ARRAY_LITERAL:
      parse_array_literal(p);
      break;
    case ASTNodeType::TY:
      parse_ty(ast_must_cast<ASTType>(p));
      break;
      ////////////////////////// declarations /////////////////////////////////
    case ASTNodeType::STRUCT_DECL:
      parse_struct_decl(p);
      break;
    case ASTNodeType::VAR_DECL:
      parse_var_decl(p);
      break;
    case ASTNodeType::ARG_DECL:
      parse_arg_decl(p);
      break;
    case ASTNodeType::FUNC_DECL:
      parse_func_decl(p);
      break;
    case ASTNodeType::ENUM_DECL:
      parse_enum_decl(p);
      break;
      /////////////////////////////// trivially parsed ASTs ///////////////////////////////////
    case ASTNodeType::BREAK:
    case ASTNodeType::CONTINUE:
    case ASTNodeType::ID:
    case ASTNodeType::INTEGER_LITERAL:
    case ASTNodeType::FLOAT_LITERAL:
    case ASTNodeType::CHAR_LITERAL:
    case ASTNodeType::STRING_LITERAL:
      ++p->_end_index;
      break;
    default:
      break;
  }
  return p->_end_index;
}

size_t ParserImpl::parse_node(const ASTBasePtr &left, const ASTBasePtr &p) {
  p->_end_index = p->_start_index;

  // FIXME: special tokens that require whether p is led or nud to determine the node type
  switch (hashed_string{p->get_token_str().c_str()}) {
    case "&"_hs:
      p->set_node_type(ASTNodeType::BAND);
      p->set_lbp(ASTBase::OpPrecedence[p->get_node_type()]);
      break;
    default:
      break;
  }

  switch (p->get_node_type()) {
    case ASTNodeType::MEMBER_ACCESS:
      parse_member_access(left, p);
      break;
    case ASTNodeType::BOP:
      parse_bop(left, p);
      break;
    case ASTNodeType::CAST: {
      ++p->_end_index; /// skip operator
      p->append_child(left); /// lhs
      auto n = next_expression(p->_end_index, p->get_lbp());
      if (!n) { error(p->_end_index, "Invalid operand"); }
      p->append_child(n);
      break;
    }
    default:
      break;
  }
  return p->_end_index;
}

ASTBasePtr ParserImpl::parse() {
  _root = Program::Create();
  parse_node(_root);
  return _root;
}

Token *ParserImpl::at(const size_t idx) const {
  if (this->eof(idx)) { report_error(_filename, _tokens.back(), "Unexpected EOF"); }
  return _tokens[idx];
}

str ParserImpl::get_filename() const { return _filename; }

bool ParserImpl::eof(size_t index) const { return index >= _tokens.size(); }

void ParserImpl::error(size_t i, const str &error_message) const { report_error(get_filename(), at(i), error_message); }

size_t ParserImpl::parse_bop(const ASTBasePtr &_lhs, const ASTBasePtr &_p) {
  ptr<Expr> lhs = ast_must_cast<Expr>(_lhs);
  ptr<BinaryOperator> p = ast_must_cast<BinaryOperator>(_p);

  ++p->_end_index; /// skip the operator

  p->set_lhs(lhs); /// lhs

  /// rhs
  auto rhs = next_expression(p->_end_index, p->get_lbp());
  if (!rhs) {
    error(p->_end_index, "Invalid operand");
  }
  p->set_rhs(rhs);

  return p->_end_index;
}

size_t ParserImpl::parse_uop(const ASTBasePtr &_p) {
  ptr<UnaryOperator> p = ast_must_cast<UnaryOperator>(_p);

  /// rhs
  ++p->_end_index;
  auto rhs = ast_cast<Expr>(next_expression(p->_end_index, p->get_lbp()));
  if (!rhs) {
    error(p->_end_index, "Invalid operand");
  }
  p->set_rhs(rhs);

  return p->_end_index;
}
