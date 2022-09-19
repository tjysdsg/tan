#include "parser.h"
#include "compiler_session.h"
#include "src/analysis/type_system.h"
#include "src/ast/stmt.h"
#include "src/ast/expr.h"
#include "src/ast/decl.h"
#include "src/ast/ast_context.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_type.h"
#include "src/common.h"
#include "src/ast/intrinsic.h"
#include "token.h"

using namespace tanlang;
using tanlang::TokenType; // distinguish from the one in winnt.h

namespace tanlang {

using nud_parsing_func_t = void (ParserImpl::*)(ASTBase *);
using led_parsing_func_t = void (ParserImpl::*)(ASTBase *, ASTBase *);

class ParserImpl final {
public:
  ParserImpl() = delete;
  explicit ParserImpl(ASTContext *ctx) : _sm(ctx->get_source_manager()), _filename(ctx->get_filename()), _cs(ctx) {}

  ASTBase *parse() {
    _root = Program::Create(SrcLoc(0));
    parse_node(_root);
    return _root;
  }

private:
  SourceManager *_sm = nullptr;
  SrcLoc _curr = SrcLoc(0);

  ASTBase *peek(TokenType type, const str &value) {
    Token *token = at(_curr);
    if (token->get_type() != type || token->get_value() != value) {
      Error err(_filename, token, fmt::format("Expect '{}' but got '{}' instead", value, token->get_value()));
      err.raise();
    }
    return peek();
  }

  ASTBase *peek_keyword(Token *token) {
    ASTBase *ret = nullptr;
    switch (hashed_string{token->get_value().c_str()}) {
      case "var"_hs:
        ret = VarDecl::Create(_curr);
        break;
      case "enum"_hs:
        ret = EnumDecl::Create(_curr);
        break;
      case "fn"_hs:
      case "pub"_hs:
      case "extern"_hs:
        ret = FunctionDecl::Create(_curr);
        break;
      case "import"_hs:
        ret = Import::Create(_curr);
        break;
      case "if"_hs:
        ret = If::Create(_curr);
        break;
        /// else clause should be covered by If statement
      case "return"_hs:
        ret = Return::Create(_curr);
        break;
      case "while"_hs:
      case "for"_hs:
        ret = Loop::Create(_curr);
        break;
      case "struct"_hs:
        ret = StructDecl::Create(_curr);
        break;
      case "break"_hs:
        ret = Break::Create(_curr);
        break;
      case "continue"_hs:
        ret = Continue::Create(_curr);
        break;
      case "as"_hs:
        ret = Cast::Create(_curr);
        break;
      case "true"_hs:
        ret = BoolLiteral::Create(_curr, true);
        break;
      case "false"_hs:
        ret = BoolLiteral::Create(_curr, false);
        break;
      default:
        return nullptr;
    }
    return ret;
  }

  ASTBase *peek() {
    if (eof(_curr)) { return nullptr; }
    Token *token = at(_curr);
    /// skip comments
    while (token && token->get_type() == TokenType::COMMENTS) {
      _curr.offset_by(1);
      if (eof(_curr)) { return nullptr; }
      token = at(_curr);
    }

    TAN_ASSERT(token);

    ASTBase *node = nullptr;
    if (token->get_value() == "@") { /// intrinsics
      node = Intrinsic::Create(_curr);
    } else if (token->get_value() == "=" && token->get_type() == TokenType::BOP) {
      node = Assignment::Create(_curr);
    } else if (token->get_value() == "!") { /// logical not
      node = UnaryOperator::Create(UnaryOpKind::LNOT, _curr);
    } else if (token->get_value() == "~") { /// binary not
      node = UnaryOperator::Create(UnaryOpKind::BNOT, _curr);
    } else if (token->get_value() == "[") {
      auto prev = _curr;
      prev.offset_by(-1);
      Token *prev_token = at(prev);
      if (prev_token->get_type() != TokenType::ID && prev_token->get_value() != "]" && prev_token->get_value() != ")") {
        /// array literal if there is no identifier, "]", or ")" before
        node = ArrayLiteral::Create(_curr);
      } else {
        /// otherwise bracket access
        node = MemberAccess::Create(_curr);
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
        case "&&"_hs:
          op = BinaryOpKind::LAND;
          break;
        case "||"_hs:
          op = BinaryOpKind::LOR;
          break;
        case "|"_hs:
          op = BinaryOpKind::BOR;
          break;
        case "&"_hs:
          op = BinaryOpKind::BAND;
          break;
        case "^"_hs:
          op = BinaryOpKind::XOR;
          break;
        default:
          error(_curr, fmt::format("Binary relational operator not implemented: {}", token->get_value().c_str()));
          return nullptr;
      }
      node = BinaryOperator::Create(op, _curr);
    } else if (token->get_type() == TokenType::INT) {
      node = IntegerLiteral::Create(_curr, (uint64_t) std::stol(token->get_value()), token->is_unsigned());
    } else if (token->get_type() == TokenType::FLOAT) {
      node = FloatLiteral::Create(_curr, std::stod(token->get_value()));
    } else if (token->get_type() == TokenType::STRING) { /// string literal
      node = StringLiteral::Create(_curr, token->get_value());
    } else if (token->get_type() == TokenType::CHAR) { /// char literal
      node = CharLiteral::Create(_curr, static_cast<uint8_t>(token->get_value()[0]));
    } else if (check_typename_token(token)) { /// types, must be before ID
      node = ASTType::Create(_cs, _curr);
    } else if (token->get_type() == TokenType::ID) {
      auto next = _curr;
      next.offset_by(1);
      Token *next_token = at(next);
      if (next_token->get_value() == "(") {
        /// identifier followed by a "(" is a function call
        node = FunctionCall::Create(_curr);
      } else {
        /// actually an identifier
        node = Identifier::Create(_curr, token->get_value());
      }
    } else if (token->get_type() == TokenType::PUNCTUATION && token->get_value() == "(") {
      node = Parenthesis::Create(_curr);
    } else if (token->get_type() == TokenType::KEYWORD) { /// keywords
      node = peek_keyword(token);
      if (!node) {
        Error err(_filename, token, "Keyword not implemented: " + token->to_string());
        err.raise();
      }
    } else if (token->get_type() == TokenType::BOP && token->get_value() == ".") { /// member access
      node = MemberAccess::Create(_curr);
    } else if (token->get_value() == "&") {
      /// BOP or UOP? ambiguous
      node = BinaryOrUnary::Create(_curr, PREC_LOWEST);
    } else if (token->get_type() == TokenType::PUNCTUATION && token->get_value() == "{") { /// statement(s)
      node = CompoundStmt::Create(_curr, true);
    } else if (token->get_type() == TokenType::BOP && check_arithmetic_token(token)) { /// arithmetic operators
      switch (hashed_string{token->get_value().c_str()}) {
        case "/"_hs:
          node = BinaryOperator::Create(BinaryOpKind::DIVIDE, _curr);
          break;
        case "%"_hs:
          node = BinaryOperator::Create(BinaryOpKind::MOD, _curr);
          break;
          /// Operators that are possibly BOP or UOP at this stage
          /// NOTE: using the precedence of the BOP form so that the parsing works correctly if it's really a BOP
        case "*"_hs:
          // MULTIPLY / PTR_DEREF
          node = BinaryOrUnary::Create(_curr, BinaryOperator::BOPPrecedence[BinaryOpKind::MULTIPLY]);
          break;
        case "+"_hs:
          // SUM / PLUS
          node = BinaryOrUnary::Create(_curr, BinaryOperator::BOPPrecedence[BinaryOpKind::SUM]);
          break;
        case "-"_hs:
          // SUBTRACT / MINUS
          node = BinaryOrUnary::Create(_curr, BinaryOperator::BOPPrecedence[BinaryOpKind::SUBTRACT]);
          break;
        default:
          TAN_ASSERT(false);
          return nullptr;
      }
    } else if (check_terminal_token(token)) { /// this MUST be the last thing to check
      return nullptr;
    } else {
      Error err(_filename, token, "Unknown token " + token->to_string());
      err.raise();
    }
    return node;
  }

  ASTBase *next_expression(int rbp) {
    ASTBase *node = peek();
    if (!node) { return nullptr; }
    auto n = node;
    parse_node(n);
    auto left = n;
    node = peek();
    if (!node) { return left; }
    while (rbp < node->get_bp()) {
      node = peek();
      n = node;
      parse_node(left, n);
      left = n;
      node = peek();
      if (!node) { break; }
    }
    return left;
  }

  /**
   * \brief Parse NUD
   */
  void parse_node(ASTBase *p) {
    /// special tokens that require whether p is led or nud to determine the node type
    if (p->get_node_type() == ASTNodeType::BOP_OR_UOP) {
      auto *pp = ast_must_cast<BinaryOrUnary>(p);
      UnaryOperator *actual = nullptr;
      str token_str = _sm->get_token_str(p->loc());
      switch (hashed_string{token_str.c_str()}) {
        case "*"_hs:
          actual = UnaryOperator::Create(UnaryOpKind::PTR_DEREF, p->loc());
          break;
        case "&"_hs:
          actual = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF, p->loc());
          break;
        case "+"_hs:
          actual = UnaryOperator::Create(UnaryOpKind::PLUS, p->loc());
          break;
        case "-"_hs:
          actual = UnaryOperator::Create(UnaryOpKind::MINUS, p->loc());
          break;
        default:
          TAN_ASSERT(false);
          break;
      }
      pp->set_uop(actual);

      // update binding power, as the value was originally set to the binding power of BOP version of this op
      parse_node(pp->get_expr_ptr());
      return;
    }

    // look up parser func from the table
    auto it = NUD_PARSING_FUNC_TABLE.find(p->get_node_type());
    if (it == NUD_PARSING_FUNC_TABLE.end()) {
      error(_curr, fmt::format("Unexpected token with type: {}", ASTBase::ASTTypeNames[p->get_node_type()]));
    }
    nud_parsing_func_t func = it->second;
    (this->*func)(p);
  }

  /**
   * \brief Parse LED
   */
  void parse_node(ASTBase *left, ASTBase *p) {
    /// special tokens that require whether p is led or nud to determine the node type
    if (p->get_node_type() == ASTNodeType::BOP_OR_UOP) {
      auto *pp = ast_must_cast<BinaryOrUnary>(p);
      BinaryOperator *actual = nullptr;
      str token_str = _sm->get_token_str(p->loc());
      switch (hashed_string{token_str.c_str()}) {
        case "*"_hs:
          actual = BinaryOperator::Create(BinaryOpKind::MULTIPLY, p->loc());
          break;
        case "&"_hs:
          actual = BinaryOperator::Create(BinaryOpKind::BAND, p->loc());
          break;
        case "+"_hs:
          actual = BinaryOperator::Create(BinaryOpKind::SUM, p->loc());
          break;
        case "-"_hs:
          actual = BinaryOperator::Create(BinaryOpKind::SUBTRACT, p->loc());
          break;
        default:
          TAN_ASSERT(false);
          break;
      }
      pp->set_bop(actual);
      parse_node(left, pp->get_expr_ptr());
      return;
    }

    // look up parser func from the table
    auto it = LED_PARSING_FUNC_TABLE.find(p->get_node_type());
    if (it == LED_PARSING_FUNC_TABLE.end()) {
      error(_curr, fmt::format("Unexpected token with type: {}", ASTBase::ASTTypeNames[p->get_node_type()]));
    }
    led_parsing_func_t func = it->second;
    (this->*func)(left, p);
  }

  [[nodiscard]] Token *at(SrcLoc loc) const {
    if (this->eof(loc)) {
      Error err(_filename, _sm->get_last_token(), "Unexpected EOF");
      err.raise();
    }
    return _sm->get_token(loc);
  }

  [[nodiscard]] bool eof(SrcLoc loc) const { return _sm->is_eof(loc); }

  [[noreturn]] void error(SrcLoc loc, const str &error_message) const {
    Error err(_filename, at(loc), error_message);
    err.raise();
  }

  Expr *expect_expression(ASTBase *p) {
    TAN_ASSERT(p);
    Expr *ret = nullptr;
    if (!(ret = ast_cast<Expr>(p))) {
      error(p->loc(), "Expect an expression");
    }
    return ret;
  }

  Stmt *expect_stmt(ASTBase *p) {
    TAN_ASSERT(p);
    Stmt *ret = nullptr;
    if (!(ret = ast_cast<Stmt>(p))) {
      error(p->loc(), "Expect a statement");
    }
    return ret;
  }

  Decl *expect_decl(ASTBase *p) {
    TAN_ASSERT(p);
    Decl *ret = nullptr;
    if (!(ret = ast_cast<Decl>(p))) {
      error(p->loc(), "Expect a declaration");
    }
    return ret;
  }

  void parse_assignment(ASTBase *left, ASTBase *_p) {
    auto p = ast_must_cast<Assignment>(_p);

    _curr.offset_by(1); /// skip =

    /// lhs
    p->set_lhs(left);

    /// rhs
    auto rhs = next_expression(PREC_LOWEST);
    p->set_rhs(expect_expression(rhs));
  }

  void parse_cast(ASTBase *left, ASTBase *_p) {
    auto lhs = ast_must_cast<Expr>(left);
    auto p = ast_must_cast<Cast>(_p);

    _curr.offset_by(1); /// skip as

    /// lhs
    p->set_lhs(lhs);

    /// rhs
    auto rhs = next_expression(p->get_bp());
    p->set_rhs(rhs);
  }

  void parse_generic_token(ASTBase *) {
    _curr.offset_by(1);
  }

  void parse_if(ASTBase *_p) {
    auto p = ast_must_cast<If>(_p);

    /// if then
    parse_if_then_branch(p);

    /// else or elif clause, if any
    while (at(_curr)->get_value() == "else") {
      _curr.offset_by(1); /// skip "else"
      if (at(_curr)->get_value() == "if") { /// elif
        parse_if_then_branch(p);
      } else { /// else
        auto else_clause = peek();
        parse_node(else_clause);
        p->add_else_branch(expect_stmt(else_clause));
      }
    }
  }

  void parse_if_then_branch(If *p) {
    _curr.offset_by(1); /// skip "if"

    /// predicate
    auto _pred = peek(TokenType::PUNCTUATION, "(");
    parse_node(_pred);
    Expr *pred = expect_expression(_pred);

    /// then clause
    auto _then = peek(TokenType::PUNCTUATION, "{");
    parse_node(_then);
    Stmt *then_clause = expect_stmt(_then);

    p->add_if_then_branch(pred, then_clause);
  }

  void parse_loop(ASTBase *_p) {
    auto p = ast_must_cast<Loop>(_p);

    if (at(_curr)->get_value() == "for") {
      // TODO: implement for loop
      p->_loop_type = ASTLoopType::FOR;
    } else if (at(_curr)->get_value() == "while") {
      p->_loop_type = ASTLoopType::WHILE;
    } else {
      TAN_ASSERT(false);
    }
    _curr.offset_by(1); /// skip while/for
    switch (p->_loop_type) {
      case ASTLoopType::WHILE: {
        /// predicate
        peek(TokenType::PUNCTUATION, "(");
        auto _pred = next_expression(PREC_LOWEST);
        Expr *pred = expect_expression(_pred);
        p->set_predicate(pred);
        peek(TokenType::PUNCTUATION, "{");

        /// loop body
        auto _body = next_expression(PREC_LOWEST);
        Stmt *body = expect_stmt(_body);
        p->set_body(body);
        break;
      }
      case ASTLoopType::FOR:
        // TODO: implement for loop
        TAN_ASSERT(false);
        break;
    }
  }

  void parse_array_literal(ASTBase *_p) {
    auto *p = ast_must_cast<ArrayLiteral>(_p);

    _curr.offset_by(1); /// skip '['

    if (at(_curr)->get_value() == "]") {
      // TODO: support empty array literal, but raise error if the type cannot be inferred
      error(p->loc(), "Empty array literal");
    }

    vector<Literal *> elements{};
    while (!eof(p->loc())) {
      if (at(_curr)->get_value() == ",") { /// skip ","
        _curr.offset_by(1);
        continue;
      } else if (at(_curr)->get_value() == "]") { /// skip "]"
        _curr.offset_by(1);
        break;
      }

      auto node = peek();
      if (!is_ast_type_in(node->get_node_type(), TypeSystem::LiteralTypes)) {
        // TODO: support array of constexpr
        error(p->loc(), "Expected a literal");
      }

      parse_node(node);
      elements.push_back(ast_must_cast<Literal>(node));
    }

    p->set_elements(elements);
  }

  void parse_bop(ASTBase *_lhs, ASTBase *_p) {
    Expr *lhs = ast_must_cast<Expr>(_lhs);

    Token *token = at(_p->loc());
    if (token->get_value() == "." || token->get_value() == "[") { /// delegate to parse_member_access
      parse_member_access(lhs, ast_must_cast<MemberAccess>(_p));
      return;
    }

    auto *p = ast_must_cast<BinaryOperator>(_p);
    _curr.offset_by(1); /// skip the operator

    p->set_lhs(lhs); /// lhs

    /// rhs
    auto rhs = next_expression(p->get_bp());
    p->set_rhs(expect_expression(rhs));
  }

  void parse_uop(ASTBase *_p) {
    auto *p = ast_must_cast<UnaryOperator>(_p);

    /// rhs
    _curr.offset_by(1);
    auto rhs = ast_cast<Expr>(next_expression(p->get_bp()));
    if (!rhs) {
      error(p->loc(), "Invalid operand");
    }
    p->set_rhs(rhs);
  }

  void parse_parenthesis(ASTBase *_p) {
    auto *p = ast_must_cast<Parenthesis>(_p);

    _curr.offset_by(1); /// skip "("
    while (true) {
      auto *t = at(_curr);
      if (t->get_type() == TokenType::PUNCTUATION && t->get_value() == ")") { /// end at )
        _curr.offset_by(1);
        break;
      }

      /// NOTE: parenthesis without child expression inside are illegal (except function call)
      auto _sub = next_expression(PREC_LOWEST);
      Expr *sub = expect_expression(_sub);
      p->set_sub(sub);
    }
  }

  void parse_func_decl(ASTBase *_p) {
    auto *p = ast_cast<FunctionDecl>(_p);

    bool is_public = false;
    bool is_external = false;

    str token_str = at(_curr)->get_value();
    if (token_str == "fn") { /// "fn"
      _curr.offset_by(1);
    } else if (token_str == "pub") { /// "pub fn"
      is_public = true;
      _curr.offset_by(2);
    } else if (token_str == "extern") { /// "extern"
      is_external = true;
      _curr.offset_by(2);
    } else {
      TAN_ASSERT(false);
    }

    /// function name
    // Don't use peek since it look ahead and returns ASTNodeType::FUNCTION when it finds "(",
    // but we only want the function name as an identifier
    // [X] auto id = peek();
    Token *id_token = at(_curr);
    auto id = Identifier::Create(_curr, id_token->get_value());
    if (id->get_node_type() != ASTNodeType::ID) {
      error(_curr, "Expect a function name");
    }
    parse_node(id);
    p->set_name(id->get_name());

    peek(TokenType::PUNCTUATION, "(");
    _curr.offset_by(1);

    /// arguments
    vector<str> arg_names{};
    vector<ASTType *> arg_types{};
    vector<ArgDecl *> arg_decls{};
    if (at(_curr)->get_value() != ")") {
      while (!eof(_curr)) {
        auto arg = ArgDecl::Create(_curr);
        parse_node(arg);

        arg_names.push_back(arg->get_name());
        arg_types.push_back(arg->get_type());
        arg_decls.push_back(arg);

        if (at(_curr)->get_value() == ",") {
          _curr.offset_by(1);
        } else {
          break;
        }
      }
    }
    peek(TokenType::PUNCTUATION, ")");
    _curr.offset_by(1);

    p->set_arg_names(arg_names);
    p->set_arg_types(arg_types);
    p->set_arg_decls(arg_decls);

    peek(TokenType::PUNCTUATION, ":");
    _curr.offset_by(1);

    /// function return type
    auto ret_type = peek();
    if (ret_type->get_node_type() != ASTNodeType::TY) {
      error(_curr, "Expect a type");
    }
    parse_node(ret_type);
    p->set_ret_type(ast_must_cast<ASTType>(ret_type));

    /// body
    if (!is_external) {
      auto body = peek(TokenType::PUNCTUATION, "{");
      parse_node(body);
      p->set_body(expect_stmt(body));
    }

    p->set_public(is_public);
    p->set_external(is_external);
  }

  void parse_func_call(ASTBase *_p) {
    auto *p = ast_must_cast<FunctionCall>(_p);

    p->set_name(at(_curr)->get_value()); /// function name
    _curr.offset_by(1);

    // No need to check since '(' is what distinguish a function call from an identifier at the first place
    // auto *token = at(_curr); if (token->get_value() != "(") { error("Invalid function call"); }
    _curr.offset_by(1); /// skip (

    /// args
    while (!eof(_curr) && at(_curr)->get_value() != ")") {
      auto _arg = next_expression(PREC_LOWEST);
      Expr *arg = expect_expression(_arg);
      p->_args.push_back(arg);

      if (at(_curr)->get_value() == ",") { /// skip ,
        _curr.offset_by(1);
      } else {
        break;
      }
    }

    peek(TokenType::PUNCTUATION, ")");
    _curr.offset_by(1);
  }

  // assuming _curr is at the token after "@"
  void parse_test_comp_error_intrinsic(Intrinsic *p) {
    _curr.offset_by(1); /// skip "test_comp_error"

    auto *e = peek();
    if (e->get_node_type() != ASTNodeType::PARENTHESIS) {
      error(_curr, "Expect a parenthesis");
    }
    parse_node(e);

    auto *test_name = ast_must_cast<Parenthesis>(e)->get_sub();
    if (test_name->get_node_type() != ASTNodeType::ID) {
      error(_curr, "Expect a test name");
    }

    // TODO: the underlying expression of this intrinsic should be Test
    // TODO: expect parsing OR analysis error

    auto *body = peek(TokenType::PUNCTUATION, "{");
    parse_node(body);
    p->set_sub(body);
    p->set_name("test_comp_error");
  }

  void parse_intrinsic(ASTBase *_p) {
    auto *p = ast_must_cast<Intrinsic>(_p);

    _curr.offset_by(1); /// skip "@"

    if (_sm->get_token_str(_curr) == "test_comp_error") {
      parse_test_comp_error_intrinsic(p);
      return;
    }

    auto e = peek();
    parse_node(e);
    /// Only allow identifier or function call as valid intrinsic token
    if (e->get_node_type() != ASTNodeType::ID && e->get_node_type() != ASTNodeType::FUNC_CALL) {
      error(_curr, "Unexpected token");
    }
    p->set_sub(e);
  }

  void parse_import(ASTBase *_p) {
    auto *p = ast_must_cast<Import>(_p);

    _curr.offset_by(1); /// skip "import"
    auto rhs = peek();
    if (rhs->get_node_type() != ASTNodeType::STRING_LITERAL) {
      error(_curr, "Invalid import statement");
    }
    parse_node(rhs);
    str filename = ast_must_cast<StringLiteral>(rhs)->get_value();
    p->set_filename(filename);
  }

  void parse_member_access(Expr *left, MemberAccess *p) {
    if (at(_curr)->get_value() == "[") {
      p->_access_type = MemberAccess::MemberAccessBracket;
    }

    _curr.offset_by(1); /// skip "." or "["

    /// lhs
    p->set_lhs(left);

    /// rhs
    auto _right = peek();
    Expr *right = expect_expression(_right);
    parse_node(right);
    p->set_rhs(right);

    if (p->_access_type == MemberAccess::MemberAccessBracket) { /// bracket access
      _curr.offset_by(1); /// skip ]
    }
  }

  void parse_program(ASTBase *_p) {
    auto *p = ast_must_cast<Program>(_p);
    while (!eof(_curr)) {
      auto stmt = CompoundStmt::Create(_curr);
      parse_node(stmt);
      p->append_child(stmt);
    }
  }

  void parse_stmt(ASTBase *_p) {
    auto p = ast_must_cast<CompoundStmt>(_p);
    if (at(_curr)->get_value() == "{") { /// compound statement
      _curr.offset_by(1); /// skip "{"
      while (!eof(_curr)) {
        auto node = peek();
        while (node) { /// stops at a terminal token
          p->append_child(next_expression(PREC_LOWEST));
          node = peek();
        }
        if (at(_curr)->get_value() == "}") {
          _curr.offset_by(1); /// skip "}"
          break;
        }
        _curr.offset_by(1);
      }
    } else { /// single statement
      auto node = peek();
      while (node) { /// stops at a terminal token
        p->append_child(next_expression(PREC_LOWEST));
        node = peek();
      }
      _curr.offset_by(1); /// skip ';'
    }
  }

  void parse_return(ASTBase *_p) {
    auto *p = ast_must_cast<Return>(_p);

    _curr.offset_by(1);

    auto _rhs = next_expression(PREC_LOWEST);
    if (_rhs) {
      Expr *rhs = expect_expression(_rhs);
      p->set_rhs(rhs);
    }
  }

  void parse_struct_decl(ASTBase *_p) {
    auto *p = ast_must_cast<StructDecl>(_p);

    _curr.offset_by(1); /// skip "struct"

    /// struct typename
    auto _id = peek();
    if (_id->get_node_type() != ASTNodeType::ID) {
      error(_curr, "Expecting a typename");
    }
    parse_node(_id);
    auto id = ast_must_cast<Identifier>(_id);
    p->set_name(id->get_name());

    /// struct body
    if (at(_curr)->get_value() == "{") {
      auto _comp_stmt = next_expression(PREC_LOWEST);
      if (!_comp_stmt || _comp_stmt->get_node_type() != ASTNodeType::STATEMENT) {
        error(_curr, "struct definition requires a valid body");
      }
      auto comp_stmt = ast_must_cast<CompoundStmt>(_comp_stmt);

      /// copy member declarations
      auto children = comp_stmt->get_children();
      vector<Expr *> member_decls{};
      for (const auto &c: children) {
        if (!is_ast_type_in(c->get_node_type(), {ASTNodeType::VAR_DECL, ASTNodeType::ASSIGN, ASTNodeType::FUNC_DECL})) {
          error(c->loc(), "Invalid struct member");
        }
        member_decls.push_back(ast_must_cast<Expr>(c));
      }
      p->set_member_decls(member_decls);
    } else {
      p->set_is_forward_decl(true);
    }
  }

  void parse_ty_array(ASTType *p) {
    bool done = false;
    while (!done) {
      /// current token should be "[" right now
      _curr.offset_by(1); /// skip "["

      /// subtype
      auto *sub = new ASTType(*p);
      p->set_ty(Ty::ARRAY);
      p->get_sub_types().clear();
      p->get_sub_types().push_back(sub);

      /// size
      ASTBase *_size = peek();
      if (_size->get_node_type() != ASTNodeType::INTEGER_LITERAL) {
        error(_curr, "Expect an unsigned integer as the array size");
      }
      parse_node(_size);

      auto size = ast_must_cast<IntegerLiteral>(_size);
      size_t array_size = size->get_value();
      if (static_cast<int64_t>(array_size) < 0) {
        error(_curr, "Expect an unsigned integer as the array size");
      }

      p->set_array_size(array_size);

      /// skip "]"
      peek(TokenType::PUNCTUATION, "]");
      _curr.offset_by(1);

      /// if followed by a "[", this is a multi-dimension array
      if (at(_curr)->get_value() != "[") {
        done = true;
      }
    }
  }

  void parse_ty(ASTBase *_p) {
    ASTType *p = ast_must_cast<ASTType>(_p);

    while (!eof(_curr)) {
      Token *token = at(_curr);
      auto qb = ASTType::BASIC_TYS.find(token->get_value());
      auto qq = ASTType::QUALIFIER_TYS.find(token->get_value());

      if (qb != ASTType::BASIC_TYS.end()) { /// base types
        p->set_ty(TY_OR(p->get_ty(), qb->second));
      } else if (qq != ASTType::QUALIFIER_TYS.end()) { /// TODO: qualifiers
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
      _curr.offset_by(1);
    }

    /// composite types
    Token *token = at(_curr);
    if (token->get_value() == "[") { /// array
      parse_ty_array(p);
    }
  }

  void parse_var_decl(ASTBase *_p) {
    auto *p = ast_must_cast<VarDecl>(_p);

    _curr.offset_by(1); /// skip 'var'

    /// name
    auto name_token = at(_curr);
    p->set_name(name_token->get_value());
    _curr.offset_by(1);

    /// type
    if (at(_curr)->get_value() == ":") {
      _curr.offset_by(1);
      ASTType *ty = ASTType::Create(_cs, _curr);
      parse_node(ty);
      p->set_type(ty);
    }
  }

  void parse_arg_decl(ASTBase *_p) {
    auto *p = ast_must_cast<ArgDecl>(_p);

    /// name
    auto name_token = at(_curr);
    p->set_name(name_token->get_value());
    _curr.offset_by(1);

    if (at(_curr)->get_value() != ":") {
      error(_curr, "Expect a type being specified");
    }
    _curr.offset_by(1);

    /// type
    ASTType *ty = ASTType::Create(_cs, _curr);
    parse_node(ty);
    p->set_type(ty);
  }

  void parse_enum_decl(ASTBase *_p) {
    auto *p = ast_must_cast<EnumDecl>(_p);

    /// skip enum
    _curr.offset_by(1);

    /// enum class name
    auto _id = peek();
    if (_id->get_node_type() != ASTNodeType::ID) { error(_curr, "Expect an identifier"); }
    parse_node(_id);
    auto id = ast_must_cast<Identifier>(_id);
    p->set_name(id->get_name());

    /// body
    if (at(_curr)->get_value() == "{") {
      auto _comp_stmt = next_expression(PREC_LOWEST);
      if (!_comp_stmt || _comp_stmt->get_node_type() != ASTNodeType::STATEMENT) {
        error(_curr, "struct definition requires a valid body");
      }
      auto comp_stmt = ast_must_cast<CompoundStmt>(_comp_stmt);

      /// copy member declarations
      auto children = comp_stmt->get_children();
      vector<Expr *> elements{};
      elements.reserve(children.size());
      for (const auto &c: children) {
        if (!is_ast_type_in(c->get_node_type(), {ASTNodeType::ASSIGN, ASTNodeType::ID})) {
          error(c->loc(), "Invalid enum elements");
        }
        elements.push_back(ast_must_cast<Expr>(c));
      }
      p->set_elements(elements);
    } else {
      // TODO: extract logic of forward declaration
      TAN_ASSERT(false);
    }
  }

private:
  str _filename;
  ASTContext *_cs = nullptr;
  ASTBase *_root = nullptr;

private:
  const static umap<ASTNodeType, nud_parsing_func_t> NUD_PARSING_FUNC_TABLE;
  const static umap<ASTNodeType, led_parsing_func_t> LED_PARSING_FUNC_TABLE;
};

Parser::Parser(ASTContext *ctx) { _impl = new ParserImpl(ctx); }

ASTBase *Parser::parse() { return _impl->parse(); }

Parser::~Parser() { delete _impl; }

const umap<ASTNodeType, nud_parsing_func_t>ParserImpl::NUD_PARSING_FUNC_TABLE =
    {{ASTNodeType::PROGRAM, &ParserImpl::parse_program}, {ASTNodeType::STATEMENT, &ParserImpl::parse_stmt},
        {ASTNodeType::PARENTHESIS, &ParserImpl::parse_parenthesis}, {ASTNodeType::IMPORT, &ParserImpl::parse_import},
        {ASTNodeType::INTRINSIC, &ParserImpl::parse_intrinsic}, {ASTNodeType::IF, &ParserImpl::parse_if},
        {ASTNodeType::LOOP, &ParserImpl::parse_loop}, {ASTNodeType::UOP, &ParserImpl::parse_uop},
        {ASTNodeType::RET, &ParserImpl::parse_return}, {ASTNodeType::FUNC_CALL, &ParserImpl::parse_func_call},
        {ASTNodeType::ARRAY_LITERAL, &ParserImpl::parse_array_literal}, {ASTNodeType::TY, &ParserImpl::parse_ty},
        {ASTNodeType::STRUCT_DECL, &ParserImpl::parse_struct_decl},
        {ASTNodeType::VAR_DECL, &ParserImpl::parse_var_decl}, {ASTNodeType::ARG_DECL, &ParserImpl::parse_arg_decl},
        {ASTNodeType::FUNC_DECL, &ParserImpl::parse_func_decl}, {ASTNodeType::ENUM_DECL, &ParserImpl::parse_enum_decl},
        {ASTNodeType::BREAK, &ParserImpl::parse_generic_token},
        {ASTNodeType::CONTINUE, &ParserImpl::parse_generic_token}, {ASTNodeType::ID, &ParserImpl::parse_generic_token},
        {ASTNodeType::INTEGER_LITERAL, &ParserImpl::parse_generic_token},
        {ASTNodeType::FLOAT_LITERAL, &ParserImpl::parse_generic_token},
        {ASTNodeType::CHAR_LITERAL, &ParserImpl::parse_generic_token},
        {ASTNodeType::STRING_LITERAL, &ParserImpl::parse_generic_token},
        {ASTNodeType::BOOL_LITERAL, &ParserImpl::parse_generic_token}};
}

const umap<ASTNodeType, led_parsing_func_t>ParserImpl::LED_PARSING_FUNC_TABLE =
    {{ASTNodeType::BOP, &ParserImpl::parse_bop}, {ASTNodeType::ASSIGN, &ParserImpl::parse_assignment},
        {ASTNodeType::CAST, &ParserImpl::parse_cast}};
