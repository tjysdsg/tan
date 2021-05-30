#include "base.h"
#include "compiler_session.h"
#include "src/parser/parser_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/stmt.h"
#include "src/ast/expr.h"
#include "src/ast/decl.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_type.h"
#include "src/common.h"
#include "src/ast/intrinsic.h"
#include "src/ast/ast_base.h"
#include "token.h"
#include <memory>
#include <utility>

using namespace tanlang;
using tanlang::TokenType; // distinguish from the one in winnt.h


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
      ret = Import::Create();
      break;
    case "if"_hs:
      ret = If::Create();
      break;
      /// else clause should be covered by If statement
    case "return"_hs:
      ret = Return::Create();
      break;
    case "while"_hs:
    case "for"_hs:
      ret = Loop::Create();
      break;
    case "struct"_hs:
      ret = StructDecl::Create();
      break;
    case "break"_hs:
      ret = Break::Create();
      break;
    case "continue"_hs:
      ret = Continue::Create();
      break;
    case "as"_hs:
      ret = BinaryOperator::Create(BinaryOpKind::CAST);
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
    node = Intrinsic::Create();
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
      node = MemberAccess::Create();
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
      node = FunctionCall::Create();
    } else {
      node = Identifier::Create(token->value);
    }
  } else if (token->type == TokenType::PUNCTUATION && token->value == "(") {
    node = Parenthesis::Create();
  } else if (token->type == TokenType::KEYWORD) { /// keywords
    node = peek_keyword(token, index);
    if (!node) { report_error(_filename, token, "Keyword not implemented: " + token->to_string()); }
  } else if (token->type == TokenType::BOP && token->value == ".") { /// member access
    node = MemberAccess::Create();
  } else if (token->value == "&") {
    /// BOP or UOP? ambiguous
    node = make_ptr<Expr>(ASTNodeType::BOP_OR_UOP, 0);
  } else if (token->type == TokenType::PUNCTUATION && token->value == "{") { /// statement(s)
    node = CompoundStmt::Create();
  } else if (token->type == TokenType::BOP && check_arithmetic_token(token)) { /// arithmetic operators
    switch (hashed_string{token->value.c_str()}) {
      case "*"_hs:
        node = BinaryOperator::Create(BinaryOpKind::MULTIPLY);
        break;
      case "/"_hs:
        node = BinaryOperator::Create(BinaryOpKind::DIVIDE);
        break;
      case "%"_hs:
        node = BinaryOperator::Create(BinaryOpKind::MOD);
        break;
      case "+"_hs:
      case "-"_hs:
        /// BOP or UOP? ambiguous
        node = make_ptr<Expr>(ASTNodeType::BOP_OR_UOP, 0);
        break;
      default:
        return nullptr;
    }
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

  /// special tokens that require whether p is led or nud to determine the node type
  if (p->get_node_type() == ASTNodeType::BOP_OR_UOP) {
    auto &pp = const_cast<ASTBasePtr &>(p); // hack
    switch (hashed_string{p->get_token_str().c_str()}) {
      case "&"_hs:
        pp = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF);
        break;
      case "+"_hs:
        pp = UnaryOperator::Create(UnaryOpKind::PLUS);
        break;
      case "-"_hs:
        pp = UnaryOperator::Create(UnaryOpKind::MINUS);
        break;
      default:
        TAN_ASSERT(false);
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
    case ASTNodeType::PARENTHESIS:
      parse_parenthesis(p);
      break;
    case ASTNodeType::IMPORT:
      parse_import(p);
      break;
    case ASTNodeType::INTRINSIC:
      parse_intrinsic(p);
      break;
    case ASTNodeType::IF:
      parse_if(p);
      break;
    case ASTNodeType::LOOP:
      parse_loop(p);
      break;
    case ASTNodeType::UOP:
      parse_uop(p);
      break;
    case ASTNodeType::RET:
      parse_return(p);
      break;
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
      // parse_enum_decl(p);
      TAN_ASSERT(false);
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

  /// special tokens that require whether p is led or nud to determine the node type
  if (p->get_node_type() == ASTNodeType::BOP_OR_UOP) {
    auto &pp = const_cast<ASTBasePtr &>(p); // hack
    switch (hashed_string{p->get_token_str().c_str()}) {
      case "&"_hs:
        pp = BinaryOperator::Create(BinaryOpKind::BAND);
        break;
      case "+"_hs:
        pp = BinaryOperator::Create(BinaryOpKind::SUM);
        break;
      case "-"_hs:
        pp = BinaryOperator::Create(BinaryOpKind::SUBTRACT);
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
  }

  switch (p->get_node_type()) {
    case ASTNodeType::BOP:
      parse_bop(left, p);
      break;
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

ExprPtr ParserImpl::expect_expression(const ASTBasePtr &p) {
  ExprPtr ret = nullptr;
  if (!(ret = ast_cast<Expr>(p))) {
    error(p->_end_index, "Expect an expression");
  }
  return ret;
}

StmtPtr ParserImpl::expect_stmt(const ASTBasePtr &p) {
  StmtPtr ret = nullptr;
  if (!(ret = ast_cast<Stmt>(p))) {
    error(p->_end_index, "Expect a statement");
  }
  return ret;
}

DeclPtr ParserImpl::expect_decl(const ASTBasePtr &p) {
  DeclPtr ret = nullptr;
  if (!(ret = ast_cast<Decl>(p))) {
    error(p->_end_index, "Expect a declaration");
  }
  return ret;
}
