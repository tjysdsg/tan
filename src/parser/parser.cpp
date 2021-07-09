#include "parser.h"
#include "base.h"
#include "compiler_session.h"
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
#include <fmt/core.h>

using namespace tanlang;
using tanlang::TokenType; // distinguish from the one in winnt.h

namespace tanlang {

class ParserImpl final {
public:
  ParserImpl() = delete;
  ParserImpl(vector<Token *> tokens, str filename, CompilerSession *cs)
      : _tokens(std::move(tokens)), _filename(std::move(filename)), _cs(cs) {}

  ASTBase *parse() {
    _root = Program::Create();
    parse_node(_root);
    return _root;
  }

  [[nodiscard]] str get_filename() const { return _filename; }

private:
  // FIXME: parse_* functions don't have to return _end_index
  //  But remember to set the _end_index of proxy classes like BinaryOrUnary
  // TODO: size_t parse_enum_decl(const ASTBase * &p);

  ASTBase *peek(size_t &index, TokenType type, const str &value) {
    if (index >= _tokens.size()) { report_error(_filename, _tokens.back(), "Unexpected EOF"); }
    Token *token = _tokens[index];
    if (token->get_type() != type || token->get_value() != value) {
      report_error(_filename, token, "Expect '" + value + "', but got '" + token->get_value() + "' instead");
    }
    return peek(index);
  }

  ASTBase *peek_keyword(Token *token, size_t &index) {
    ASTBase *ret = nullptr;
    switch (hashed_string{token->get_value().c_str()}) {
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
        ret = Cast::Create();
        break;
      default:
        return nullptr;
    }
    ret->set_token(token);
    ret->_start_index = index;
    return ret;
  }

  ASTBase *peek(size_t &index) {
    if (index >= _tokens.size()) { return nullptr; }
    Token *token = _tokens[index];
    /// skip comments
    while (token && token->get_type() == TokenType::COMMENTS) {
      ++index;
      token = _tokens[index];
    }
    // check if there are tokens after the comment
    if (index >= _tokens.size()) { return nullptr; }

    ASTBase *node = nullptr;
    if (token->get_value() == "@") { /// intrinsics
      node = Intrinsic::Create();
    } else if (token->get_value() == "=" && token->get_type() == TokenType::BOP) {
      node = Assignment::Create();
    } else if (token->get_value() == "!") { /// logical not
      node = UnaryOperator::Create(UnaryOpKind::LNOT);
    } else if (token->get_value() == "~") { /// binary not
      node = UnaryOperator::Create(UnaryOpKind::BNOT);
    } else if (token->get_value() == "[") {
      auto prev = this->at(index - 1);
      if (prev->get_type() != TokenType::ID && prev->get_value() != "]" && prev->get_value() != ")") {
        /// array literal if there is no identifier, "]", or ")" before
        node = ArrayLiteral::Create();
      } else {
        /// otherwise bracket access
        node = MemberAccess::Create();
      }
    } else if (token->get_type() == TokenType::RELOP) { /// comparisons
      BinaryOpKind op = BinaryOpKind::INVALID;
      switch (hashed_string{token->get_value().c_str()}) {
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
    } else if (token->get_type() == TokenType::INT) {
      node = IntegerLiteral::Create((uint64_t) std::stol(token->get_value()), token->is_unsigned());
    } else if (token->get_type() == TokenType::FLOAT) {
      node = FloatLiteral::Create(std::stod(token->get_value()));
    } else if (token->get_type() == TokenType::STRING) { /// string literal
      node = StringLiteral::Create(token->get_value());
    } else if (token->get_type() == TokenType::CHAR) { /// char literal
      node = CharLiteral::Create(static_cast<uint8_t>(token->get_value()[0]));
    } else if (check_typename_token(token)) { /// types, must be before ID
      node = ASTType::Create(_cs);
    } else if (token->get_type() == TokenType::ID) {
      Token *next = _tokens[index + 1];
      if (next->get_value() == "(") {
        node = FunctionCall::Create();
      } else {
        node = Identifier::Create(token->get_value());
      }
    } else if (token->get_type() == TokenType::PUNCTUATION && token->get_value() == "(") {
      node = Parenthesis::Create();
    } else if (token->get_type() == TokenType::KEYWORD) { /// keywords
      node = peek_keyword(token, index);
      if (!node) { report_error(_filename, token, "Keyword not implemented: " + token->to_string()); }
    } else if (token->get_type() == TokenType::BOP && token->get_value() == ".") { /// member access
      node = MemberAccess::Create();
    } else if (token->get_value() == "&") {
      /// BOP or UOP? ambiguous
      node = BinaryOrUnary::Create(PREC_LOWEST);
    } else if (token->get_type() == TokenType::PUNCTUATION && token->get_value() == "{") { /// statement(s)
      node = CompoundStmt::Create();
    } else if (token->get_type() == TokenType::BOP && check_arithmetic_token(token)) { /// arithmetic operators
      switch (hashed_string{token->get_value().c_str()}) {
        case "/"_hs:
          node = BinaryOperator::Create(BinaryOpKind::DIVIDE);
          break;
        case "%"_hs:
          node = BinaryOperator::Create(BinaryOpKind::MOD);
          break;
          /// Operators that are possibly BOP or UOP at this stage
          /// NOTE: using the precedence of the BOP form so that the parsing works correctly if it's really a BOP
        case "*"_hs:
          // MULTIPLY / PTR_DEREF
          node = BinaryOrUnary::Create(BinaryOperator::BOPPrecedence[BinaryOpKind::MULTIPLY]);
          break;
        case "+"_hs:
          // SUM / PLUS
          node = BinaryOrUnary::Create(BinaryOperator::BOPPrecedence[BinaryOpKind::SUM]);
          break;
        case "-"_hs:
          // SUBTRACT / MINUS
          node = BinaryOrUnary::Create(BinaryOperator::BOPPrecedence[BinaryOpKind::SUBTRACT]);
          break;
        default:
          TAN_ASSERT(false);
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

  ASTBase *next_expression(size_t &index, int rbp) {
    ASTBase *node = peek(index);
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

  size_t parse_node(ASTBase *p) {
    p->_end_index = p->_start_index;

    /// special tokens that require whether p is led or nud to determine the node type
    if (p->get_node_type() == ASTNodeType::BOP_OR_UOP) {
      BinaryOrUnary *pp = ast_must_cast<BinaryOrUnary>(p);
      UnaryOperator *actual = nullptr;
      switch (hashed_string{p->get_token_str().c_str()}) {
        case "*"_hs:
          actual = UnaryOperator::Create(UnaryOpKind::PTR_DEREF);
          break;
        case "&"_hs:
          actual = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF);
          break;
        case "+"_hs:
          actual = UnaryOperator::Create(UnaryOpKind::PLUS);
          break;
        case "-"_hs:
          actual = UnaryOperator::Create(UnaryOpKind::MINUS);
          break;
        default:
          TAN_ASSERT(false);
          break;
      }
      actual->_start_index = p->_start_index;
      actual->_end_index = p->_end_index;
      actual->set_token(p->get_token());
      pp->set_uop(actual);
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
      case ASTNodeType::BREAK:
      case ASTNodeType::CONTINUE:
      case ASTNodeType::ID:
      case ASTNodeType::INTEGER_LITERAL:
      case ASTNodeType::FLOAT_LITERAL:
      case ASTNodeType::CHAR_LITERAL:
      case ASTNodeType::STRING_LITERAL:
        ++p->_end_index;
        break;
      case ASTNodeType::BOP_OR_UOP: {
        BinaryOrUnary *pp = ast_must_cast<BinaryOrUnary>(p);
        TAN_ASSERT(pp->get_kind() == BinaryOrUnary::UNARY);

        /// update binding power, as the value was originally set to the binding power of BOP version of this op
        auto *uop = pp->get_uop();
        uop->set_lbp(UnaryOperator::UOPPrecedence[uop->get_op()]);

        p->_end_index = parse_node(pp->get_generic_ptr());
        break;
      }
      default:
        error(p->_end_index, fmt::format("Unexpected token with type: {}", ASTBase::ASTTypeNames[p->get_node_type()]));
        break;
    }
    return p->_end_index;
  }

  size_t parse_node(ASTBase *left, ASTBase *p) {
    p->_end_index = p->_start_index;

    /// special tokens that require whether p is led or nud to determine the node type
    if (p->get_node_type() == ASTNodeType::BOP_OR_UOP) {
      BinaryOrUnary *pp = ast_must_cast<BinaryOrUnary>(p);
      BinaryOperator *actual = nullptr;
      switch (hashed_string{p->get_token_str().c_str()}) {
        case "*"_hs:
          actual = BinaryOperator::Create(BinaryOpKind::MULTIPLY);
          break;
        case "&"_hs:
          actual = BinaryOperator::Create(BinaryOpKind::BAND);
          break;
        case "+"_hs:
          actual = BinaryOperator::Create(BinaryOpKind::SUM);
          break;
        case "-"_hs:
          actual = BinaryOperator::Create(BinaryOpKind::SUBTRACT);
          break;
        default:
          TAN_ASSERT(false);
          break;
      }
      actual->_start_index = p->_start_index;
      actual->_end_index = p->_end_index;
      actual->set_token(p->get_token());
      pp->set_bop(actual);
    }

    switch (p->get_node_type()) {
      case ASTNodeType::BOP:
        parse_bop(left, p);
        break;
      case ASTNodeType::ASSIGN:
        parse_assignment(left, p);
        break;
      case ASTNodeType::CAST:
        parse_cast(left, p);
        break;
      case ASTNodeType::BOP_OR_UOP: {
        BinaryOrUnary *pp = ast_must_cast<BinaryOrUnary>(p);
        TAN_ASSERT(pp->get_kind() == BinaryOrUnary::BINARY);
        p->_end_index = parse_node(left, pp->get_generic_ptr());
        break;
      }
      default:
        TAN_ASSERT(false);
        break;
    }
    return p->_end_index;
  }

  [[nodiscard]] Token *at(const size_t idx) const {
    if (this->eof(idx)) { report_error(_filename, _tokens.back(), "Unexpected EOF"); }
    return _tokens[idx];
  }

  bool eof(size_t index) const { return index >= _tokens.size(); }

  [[noreturn]] void error(size_t i, const str &error_message) const {
    report_error(get_filename(), at(i), error_message);
  }

  Expr *expect_expression(ASTBase *p) {
    Expr *ret = nullptr;
    if (!(ret = ast_cast<Expr>(p))) {
      error(p->_end_index, "Expect an expression");
    }
    return ret;
  }

  Stmt *expect_stmt(ASTBase *p) {
    Stmt *ret = nullptr;
    if (!(ret = ast_cast<Stmt>(p))) {
      error(p->_end_index, "Expect a statement");
    }
    return ret;
  }

  Decl *expect_decl(ASTBase *p) {
    Decl *ret = nullptr;
    if (!(ret = ast_cast<Decl>(p))) {
      error(p->_end_index, "Expect a declaration");
    }
    return ret;
  }

  size_t parse_assignment(ASTBase *left, ASTBase *_p) {
    auto p = ast_must_cast<Assignment>(_p);

    ++p->_end_index; /// skip =

    /// lhs
    p->set_lhs(left);

    /// rhs
    auto rhs = next_expression(p->_end_index, PREC_LOWEST);
    p->set_rhs(expect_expression(rhs));

    return p->_end_index;
  }

  size_t parse_cast(ASTBase *left, ASTBase *_p) {
    auto lhs = ast_must_cast<Expr>(left);
    auto p = ast_must_cast<Cast>(_p);

    ++p->_end_index; /// skip as

    /// lhs
    p->set_lhs(lhs);

    /// rhs
    auto rhs = next_expression(p->_end_index, p->get_lbp());
    if (rhs->get_node_type() != ASTNodeType::TY) {
      error(rhs->_end_index, "Expect a type");
    }
    p->set_rhs(rhs);

    return p->_end_index;
  }

  size_t parse_if(ASTBase *_p) {
    auto p = ast_must_cast<If>(_p);

    /// if then
    parse_if_then_branch(p);

    /// else or elif clause, if any
    while (at(p->_end_index)->get_value() == "else") {
      ++p->_end_index; /// skip "else"
      if (at(p->_end_index)->get_value() == "if") { /// elif
        p->_end_index = parse_if_then_branch(p);
      } else { /// else
        auto else_clause = peek(p->_end_index);
        p->_end_index = parse_node(else_clause);
        p->add_else_branch(expect_stmt(else_clause));
      }
    }
    return p->_end_index;
  }

  size_t parse_if_then_branch(If *p) {
    ++p->_end_index; /// skip "if"

    /// predicate
    auto _pred = peek(p->_end_index, TokenType::PUNCTUATION, "(");
    p->_end_index = parse_node(_pred);
    Expr *pred = expect_expression(_pred);

    /// then clause
    auto _then = peek(p->_end_index, TokenType::PUNCTUATION, "{");
    p->_end_index = parse_node(_then);
    Stmt *then_clause = expect_stmt(_then);

    p->add_if_then_branch(pred, then_clause);
    return p->_end_index;
  }

  size_t parse_loop(ASTBase *_p) {
    auto p = ast_must_cast<Loop>(_p);

    if (at(p->_end_index)->get_value() == "for") {
      // TODO: implement for loop
      p->_loop_type = ASTLoopType::FOR;
    } else if (at(p->_end_index)->get_value() == "while") {
      p->_loop_type = ASTLoopType::WHILE;
    } else {
      TAN_ASSERT(false);
    }
    ++p->_end_index; /// skip while/for
    switch (p->_loop_type) {
      case ASTLoopType::WHILE: {
        /// predicate
        peek(p->_end_index, TokenType::PUNCTUATION, "(");
        auto _pred = next_expression(p->_end_index, PREC_LOWEST);
        Expr *pred = expect_expression(_pred);
        p->set_predicate(pred);
        peek(p->_end_index, TokenType::PUNCTUATION, "{");

        /// loop body
        auto _body = next_expression(p->_end_index, PREC_LOWEST);
        Stmt *body = expect_stmt(_body);
        p->set_body(body);
        break;
      }
      case ASTLoopType::FOR:
        // TODO: implement for loop
        TAN_ASSERT(false);
        break;
    }
    return p->_end_index;
  }

  // TODO: move type checking of array elements to analysis phase
  size_t parse_array_literal(ASTBase *_p) {
    ArrayLiteral *p = ast_must_cast<ArrayLiteral>(_p);

    ++p->_end_index; /// skip '['

    if (at(p->_end_index)->get_value() == "]") {
      // TODO: support empty array literal, but raise error if the type cannot be inferred
      error(p->_end_index, "Empty array literal");
    }

    auto element_type = ASTNodeType::INVALID;
    vector<Literal *> elements{};
    while (!eof(p->_end_index)) {
      if (at(p->_end_index)->get_value() == ",") { /// skip ","
        ++p->_end_index;
        continue;
      } else if (at(p->_end_index)->get_value() == "]") { /// skip "]"
        ++p->_end_index;
        break;
      }

      auto node = peek(p->_end_index);
      if (!is_ast_type_in(node->get_node_type(), TypeSystem::LiteralTypes)) {
        // TODO: support array of constexpr
        error(p->_end_index, "Expected a literal");
      }

      if (element_type == ASTNodeType::INVALID) { /// set the element type to first element if unknown
        element_type = node->get_node_type();
      } else { /// otherwise check whether element types are the same
        if (element_type != node->get_node_type()) {
          error(p->_end_index, "All elements in an array must have the same type");
        }
      }
      p->_end_index = parse_node(node);
      elements.push_back(ast_must_cast<Literal>(node));
    }

    p->set_elements(elements);
    return p->_end_index;
  }

  size_t parse_bop(ASTBase *_lhs, ASTBase *_p) {
    Expr *lhs = ast_must_cast<Expr>(_lhs);

    if (_p->get_token_str() == "." || _p->get_token_str() == "[") { /// delegate to parse_member_access
      return parse_member_access(lhs, ast_must_cast<MemberAccess>(_p));
    }

    BinaryOperator *p = ast_must_cast<BinaryOperator>(_p);
    ++p->_end_index; /// skip the operator

    p->set_lhs(lhs); /// lhs

    /// rhs
    auto rhs = next_expression(p->_end_index, p->get_lbp());
    p->set_rhs(expect_expression(rhs));

    return p->_end_index;
  }

  size_t parse_uop(ASTBase *_p) {
    UnaryOperator *p = ast_must_cast<UnaryOperator>(_p);

    /// rhs
    ++p->_end_index;
    auto rhs = ast_cast<Expr>(next_expression(p->_end_index, p->get_lbp()));
    if (!rhs) {
      error(p->_end_index, "Invalid operand");
    }
    p->set_rhs(rhs);

    return p->_end_index;
  }

  size_t parse_parenthesis(ASTBase *_p) {
    Parenthesis *p = ast_must_cast<Parenthesis>(_p);

    ++p->_end_index; /// skip "("
    while (true) {
      auto *t = at(p->_end_index);
      if (!t) {
        error(p->_end_index - 1, "Unexpected EOF");
      } else if (t->get_type() == TokenType::PUNCTUATION && t->get_value() == ")") { /// end at )
        ++p->_end_index;
        break;
      }
      // FIXME: multiple expressions in the parenthesis?

      /// NOTE: parenthesis without child expression inside are illegal (except function call)
      auto _sub = next_expression(p->_end_index, PREC_LOWEST);
      Expr *sub = expect_expression(_sub);
      p->set_sub(sub);
    }
    return 0;
  }

  size_t parse_func_decl(ASTBase *_p) {
    FunctionDecl *p = ast_cast<FunctionDecl>(_p);

    bool is_public = false;
    bool is_external = false;

    if (at(p->_start_index)->get_value() == "fn") { /// "fn"
      ++p->_end_index;
    } else if (at(p->_start_index)->get_value() == "pub") { /// "pub fn"
      is_public = true;
      p->_end_index = p->_start_index + 2;
    } else if (at(p->_start_index)->get_value() == "extern") { /// "extern"
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
    auto id = Identifier::Create(id_token->get_value());
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
    vector<ASTType *> arg_types{};
    vector<ArgDecl *> arg_decls{};
    if (at(p->_end_index)->get_value() != ")") {
      while (!eof(p->_end_index)) {
        auto arg = ArgDecl::Create();
        arg->set_token(at(p->_end_index));
        arg->_end_index = arg->_start_index = p->_end_index;
        p->_end_index = parse_node(arg);

        arg_names.push_back(arg->get_name());
        arg_types.push_back(arg->get_type());
        arg_decls.push_back(arg);

        if (at(p->_end_index)->get_value() == ",") {
          ++p->_end_index;
        } else {
          break;
        }
      }
    }
    peek(p->_end_index, TokenType::PUNCTUATION, ")");
    ++p->_end_index;

    p->set_arg_names(arg_names);
    p->set_arg_types(arg_types);
    p->set_arg_decls(arg_decls);

    peek(p->_end_index, TokenType::PUNCTUATION, ":");
    ++p->_end_index;

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

    p->set_public(is_public);
    p->set_external(is_external);
    return p->_end_index;
  }

  size_t parse_func_call(ASTBase *_p) {
    FunctionCall *p = ast_must_cast<FunctionCall>(_p);

    p->set_name(at(p->_end_index)->get_value()); /// function name
    ++p->_end_index;

    // No need to check since '(' is what distinguish a function call from an identifier at the first place
    // auto *token = at(p->_end_index); if (token->get_value() != "(") { error("Invalid function call"); }
    ++p->_end_index; /// skip (

    /// args
    while (!eof(p->_end_index) && at(p->_end_index)->get_value() != ")") {
      auto _arg = next_expression(p->_end_index, PREC_LOWEST);
      Expr *arg = expect_expression(_arg);
      p->_args.push_back(arg);

      if (at(p->_end_index)->get_value() == ",") { /// skip ,
        ++p->_end_index;
      } else {
        break;
      }
    }

    peek(p->_end_index, TokenType::PUNCTUATION, ")");
    ++p->_end_index;

    return p->_end_index;
  }

  size_t parse_intrinsic(ASTBase *_p) {
    Intrinsic *p = ast_must_cast<Intrinsic>(_p);

    ++p->_end_index; /// skip "@"
    auto e = peek(p->_end_index);
    p->_end_index = parse_node(e);
    /// Only allow identifier or function call as valid intrinsic token
    if (e->get_node_type() != ASTNodeType::ID && e->get_node_type() != ASTNodeType::FUNC_CALL) {
      error(e->_end_index, "Unexpected token");
    }
    p->set_sub(e);
    return p->_end_index;
  }

  size_t parse_import(ASTBase *_p) {
    Import *p = ast_must_cast<Import>(_p);

    ++p->_end_index; /// skip "import"
    auto rhs = peek(p->_end_index);
    if (rhs->get_node_type() != ASTNodeType::STRING_LITERAL) {
      error(p->_end_index, "Invalid import statement");
    }
    p->_end_index = parse_node(rhs);
    str filename = ast_must_cast<StringLiteral>(rhs)->get_value();
    p->set_filename(filename);
    return p->_end_index;
  }

  size_t parse_member_access(Expr *left, MemberAccess *p) {
    if (at(p->_end_index)->get_value() == "[") {
      p->_access_type = MemberAccess::MemberAccessBracket;
    }

    ++p->_end_index; /// skip "." or "["

    /// lhs
    p->set_lhs(left);

    /// rhs
    auto _right = peek(p->_end_index);
    Expr *right = expect_expression(_right);
    p->_end_index = parse_node(right);
    p->set_rhs(right);

    if (p->_access_type == MemberAccess::MemberAccessBracket) { /// bracket access
      ++p->_end_index; /// skip ]
    } else if (p->_access_type != MemberAccess::MemberAccessBracket
        && right->get_token_str() == "*") { /// pointer dereference
      p->_access_type = MemberAccess::MemberAccessDeref;
      ++p->_end_index; // skip *
    } else if (right->get_node_type() == ASTNodeType::FUNC_CALL) { /// method call
      p->_access_type = MemberAccess::MemberAccessMemberFunction;
    }

    if (!(p->_access_type == MemberAccess::MemberAccessBracket
        || p->_access_type == MemberAccess::MemberAccessMemberFunction
        || p->_access_type == MemberAccess::MemberAccessDeref /// pointer dereference
        || right->get_node_type() == ASTNodeType::ID /// member variable or enum
    )) {
      error(right->_end_index, "Invalid right-hand operand");
    }

    return p->_end_index;
  }

  size_t parse_program(ASTBase *_p) {
    Program *p = ast_must_cast<Program>(_p);
    while (!eof(p->_end_index)) {
      auto stmt = CompoundStmt::Create();
      stmt->set_token(at(p->_end_index));
      stmt->_start_index = p->_end_index;
      p->_end_index = parse_node(stmt);
      p->append_child(stmt);
    }
    return p->_end_index;
  }

  size_t parse_stmt(ASTBase *_p) {
    auto p = ast_must_cast<CompoundStmt>(_p);
    if (at(p->_end_index)->get_value() == "{") { /// compound statement
      ++p->_end_index; /// skip "{"
      while (!eof(p->_end_index)) {
        auto node = peek(p->_end_index);
        while (node) { /// stops at a terminal token
          p->append_child(next_expression(p->_end_index, PREC_LOWEST));
          node = peek(p->_end_index);
        }
        if (at(p->_end_index)->get_value() == "}") {
          ++p->_end_index; /// skip "}"
          break;
        }
        ++p->_end_index;
      }
    } else { /// single statement
      auto node = peek(p->_end_index);
      while (node) { /// stops at a terminal token
        p->append_child(next_expression(p->_end_index, PREC_LOWEST));
        node = peek(p->_end_index);
      }
      ++p->_end_index; /// skip ';'
    }
    return p->_end_index;
  }

  size_t parse_return(ASTBase *_p) {
    Return *p = ast_must_cast<Return>(_p);

    ++p->_end_index;

    auto _rhs = next_expression(p->_end_index, PREC_LOWEST);
    Expr *rhs = expect_expression(_rhs);
    p->set_rhs(rhs);
    return p->_end_index;
  }

  size_t parse_struct_decl(ASTBase *_p) {
    StructDecl *p = ast_must_cast<StructDecl>(_p);

    ++p->_end_index; /// skip "struct"

    /// struct typename
    auto _id = peek(p->_end_index);
    if (_id->get_node_type() != ASTNodeType::ID) {
      error(p->_end_index, "Expecting a typename");
    }
    p->_end_index = parse_node(_id);
    auto id = ast_must_cast<Identifier>(_id);
    p->set_name(id->get_name());

    /// struct body
    if (at(p->_end_index)->get_value() == "{") {
      auto _comp_stmt = next_expression(p->_end_index, PREC_LOWEST);
      if (!_comp_stmt || _comp_stmt->get_node_type() != ASTNodeType::STATEMENT) {
        error(p->_end_index, "struct definition requires a valid body");
      }
      auto comp_stmt = ast_must_cast<CompoundStmt>(_comp_stmt);

      /// copy member declarations
      auto children = comp_stmt->get_children();
      vector<Expr *> member_decls{};
      for (const auto &c : children) {
        if (!is_ast_type_in(c->get_node_type(), {ASTNodeType::VAR_DECL, ASTNodeType::ASSIGN, ASTNodeType::FUNC_DECL})) {
          error(c->_end_index, "Invalid struct member");
        }
        member_decls.push_back(ast_must_cast<Expr>(c));
      }
      p->set_member_decls(member_decls);
    } else {
      p->set_is_forward_decl(true);
    }

    return p->_end_index;
  }

  size_t parse_ty_array(ASTType *p) {
    bool done = false;
    while (!done) {
      /// current token should be "[" right now
      ++p->_end_index; /// skip "["

      /// subtype
      ASTType *sub = new ASTType(*p);
      p->set_ty(Ty::ARRAY);
      p->get_sub_types().clear();
      p->get_sub_types().push_back(sub);

      /// size
      ASTBase *_size = peek(p->_end_index);
      if (_size->get_node_type() != ASTNodeType::INTEGER_LITERAL) {
        error(p->_end_index, "Expect an unsigned integer as the array size");
      }
      p->_end_index = parse_node(_size);

      auto size = ast_must_cast<IntegerLiteral>(_size);
      size_t array_size = size->get_value();
      if (static_cast<int64_t>(array_size) < 0) {
        error(p->_end_index, "Expect an unsigned integer as the array size");
      }

      p->set_array_size(array_size);

      /// skip "]"
      peek(p->_end_index, TokenType::PUNCTUATION, "]");
      ++p->_end_index;

      /// if followed by a "[", this is a multi-dimension array
      if (at(p->_end_index)->get_value() != "[") {
        done = true;
      }
    }
    return p->_end_index;
  }

  size_t parse_ty(ASTType *p) {
    while (!eof(p->_end_index)) {
      Token *token = at(p->_end_index);
      auto qb = ASTType::basic_tys.find(token->get_value());
      auto qq = ASTType::qualifier_tys.find(token->get_value());

      if (qb != ASTType::basic_tys.end()) { /// base types
        p->set_ty(TY_OR(p->get_ty(), qb->second));
      } else if (qq != ASTType::qualifier_tys.end()) { /// TODO: qualifiers
        if (token->get_value() == "*") { /// pointer
          auto sub = new ASTType(*p);
          p->set_ty(Ty::POINTER);
          p->get_sub_types().clear();
          p->get_sub_types().push_back(sub);
        }
      } else if (token->get_type() == TokenType::ID) { /// struct or enum
        /// type is resolved in analysis phase
        p->set_ty(Ty::TYPE_REF);
      } else {
        break;
      }
      ++p->_end_index;
    }

    /// composite types
    Token *token = at(p->_end_index);
    if (token->get_value() == "[") { /// array
      p->_end_index = parse_ty_array(p);
    }
    return p->_end_index;
  }

  size_t parse_var_decl(ASTBase *_p) {
    VarDecl *p = ast_must_cast<VarDecl>(_p);

    ++p->_end_index; /// skip 'var'

    /// name
    auto name_token = at(p->_end_index);
    p->set_name(name_token->get_value());
    ++p->_end_index;

    /// type
    if (at(p->_end_index)->get_value() == ":") {
      ++p->_end_index;
      ASTType *ty = ASTType::Create(_cs);
      ty->set_token(at(p->_end_index));
      ty->_end_index = ty->_start_index = p->_end_index;
      p->_end_index = parse_node(ty);
      p->set_type(ty);
    }

    return p->_end_index;
  }

  size_t parse_arg_decl(ASTBase *_p) {
    ArgDecl *p = ast_must_cast<ArgDecl>(_p);

    /// name
    auto name_token = at(p->_end_index);
    p->set_name(name_token->get_value());
    ++p->_end_index;

    if (at(p->_end_index)->get_value() != ":") {
      error(p->_end_index, "Expect a type being specified");
    }
    ++p->_end_index;

    /// type
    ASTType *ty = ASTType::Create(_cs);
    ty->set_token(at(p->_end_index));
    ty->_end_index = ty->_start_index = p->_end_index;
    p->_end_index = parse_node(ty);
    p->set_type(ty);

    return p->_end_index;
  }

private:
  vector<Token *> _tokens{};
  str _filename = "";
  CompilerSession *_cs = nullptr;
  ASTBase *_root = nullptr;
};

}

Parser::Parser(vector<Token *> tokens, str filename, CompilerSession *cs) {
  _impl = new tanlang::ParserImpl(tokens, filename, cs);
}

ASTBase *Parser::parse() {
  return _impl->parse();
}

str Parser::get_filename() const { return _impl->get_filename(); }

Parser::~Parser() { delete _impl; }
