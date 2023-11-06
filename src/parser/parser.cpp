#include "parser/parser.h"
#include "ast/ast_node_type.h"
#include "ast/stmt.h"
#include "ast/expr.h"
#include "ast/decl.h"
#include "ast/type.h"
#include "ast/context.h"
#include "ast/intrinsic.h"
#include "source_file/token.h"
#include <iostream>

using namespace tanlang;
using tanlang::TokenType; // distinguish from the one in winnt.h

// ========= helper functions' prototypes ========= //

static bool check_typename_token(Token *token);
static bool check_terminal_token(Token *token);

namespace tanlang {

using nud_parsing_func_t = void (ParserImpl::*)(ASTBase *);
using led_parsing_func_t = void (ParserImpl::*)(ASTBase *, ASTBase *);

class ScopeGuard {
public:
  ScopeGuard() = delete;

  ScopeGuard(ASTBase *&variable, ASTBase *scoped_val) : _variable(variable), _original(variable) {
    _variable = scoped_val;
  }

  ~ScopeGuard() { _variable = _original; }

private:
  ASTBase *&_variable;
  ASTBase *_original;
};

class ParserImpl final {
public:
  ParserImpl() = delete;
  explicit ParserImpl(TokenizedSourceFile *src) : _src(src), _filename(src->get_filename()) {}

  Program *parse() {
    _root = Program::Create(_src->src());

    ScopeGuard scope_guard(_curr_scope, _root);

    while (!eof(_curr)) {
      ASTBase *node = next_expression(PREC_LOWEST);
      if (!node)
        error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Unexpected terminal token");

      _root->append_child(node);

      expect_token(node->terminal_token());
      ++_curr;
    }

    return _root;
  }

private:
  ASTBase *peek_keyword(Token *token) {
    ASTBase *ret = nullptr;
    str tok = token->get_value();
    if (tok == "var")
      ret = VarDecl::Create(_src->src());
    else if (tok == "fn" || tok == "pub" || tok == "extern")
      ret = FunctionDecl::Create(_src->src());
    else if (tok == "import")
      ret = Import::Create(_src->src());
    else if (tok == "if") /// else clause should be covered by If statement as well
      ret = If::Create(_src->src());
    else if (tok == "return")
      ret = Return::Create(_src->src());
    else if (tok == "while" || tok == "for")
      ret = Loop::Create(_src->src());
    else if (tok == "struct")
      ret = StructDecl::Create(_src->src());
    else if (tok == "break")
      ret = Break::Create(_src->src());
    else if (tok == "continue")
      ret = Continue::Create(_src->src());
    else if (tok == "as")
      ret = Cast::Create(_src->src());
    else if (tok == "true")
      ret = BoolLiteral::Create(_src->src(), true);
    else if (tok == "false")
      ret = BoolLiteral::Create(_src->src(), false);
    else if (tok == "package")
      ret = PackageDecl::Create(_src->src());

    TAN_ASSERT(ret);
    return ret;
  }

  Type *peek_type() {
    str s = at(_curr)->get_value();

    if (!std::isalpha(s[0])) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Invalid type name");
    }

    return Type::GetTypeRef(s); // placeholder type
  }

  ASTBase *peek() {
    if (eof(_curr)) {
      return nullptr;
    }
    Token *token = at(_curr);
    /// skip comments
    while (token && token->get_type() == TokenType::COMMENTS) {
      ++_curr;
      if (eof(_curr)) {
        return nullptr;
      }
      token = at(_curr);
    }

    TAN_ASSERT(token);

    ASTBase *node = nullptr;
    if (token->get_value() == "@") { /// intrinsics
      node = Intrinsic::Create(_src->src());
    } else if (token->get_value() == "=" && token->get_type() == TokenType::BOP) {
      node = Assignment::Create(_src->src());
    } else if (token->get_value() == "!") { /// logical not
      node = UnaryOperator::Create(UnaryOpKind::LNOT, _src->src());
    } else if (token->get_value() == "~") { /// binary not
      node = UnaryOperator::Create(UnaryOpKind::BNOT, _src->src());
    } else if (token->get_value() == "[") {
      Token *prev_token = at(_curr - 1);
      if (prev_token->get_type() != TokenType::ID && prev_token->get_value() != "]" && prev_token->get_value() != ")") {
        /// array literal if there is no identifier, "]", or ")" before
        node = ArrayLiteral::Create(_src->src());
      } else {
        /// otherwise bracket access
        node = MemberAccess::Create(_src->src());
      }
    } else if (token->get_type() == TokenType::RELOP) { /// comparisons
      BinaryOpKind op;
      str tok = token->get_value();
      if (tok == ">")
        op = BinaryOpKind::GT;
      else if (tok == ">=")
        op = BinaryOpKind::GE;
      else if (tok == "<")
        op = BinaryOpKind::LT;
      else if (tok == "<=")
        op = BinaryOpKind::LE;
      else if (tok == "==")
        op = BinaryOpKind::EQ;
      else if (tok == "!=")
        op = BinaryOpKind::NE;
      else if (tok == "&&")
        op = BinaryOpKind::LAND;
      else if (tok == "||")
        op = BinaryOpKind::LOR;
      else
        error(ErrorType::NOT_IMPLEMENTED, _curr, _curr,
              fmt::format("Binary relational operator not implemented: {}", token->get_value().c_str()));

      node = BinaryOperator::Create(op, _src->src());
    } else if (token->get_type() == TokenType::INT) {
      node = IntegerLiteral::Create(_src->src(), (uint64_t)std::stol(token->get_value()), token->is_unsigned());
    } else if (token->get_type() == TokenType::FLOAT) {
      node = FloatLiteral::Create(_src->src(), std::stod(token->get_value()));
    } else if (token->get_type() == TokenType::STRING) { /// string literal
      node = StringLiteral::Create(_src->src(), token->get_value());
    } else if (token->get_type() == TokenType::CHAR) { /// char literal
      node = CharLiteral::Create(_src->src(), static_cast<uint8_t>(token->get_value()[0]));
    } else if (check_typename_token(token)) { /// should not encounter types if parsed properly
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Unexpected type name");
    } else if (token->get_type() == TokenType::ID) {
      auto next = _curr;
      Token *next_token = at(++next);
      if (next_token->get_value() == "(") {
        /// identifier followed by a "(" is a function call
        node = FunctionCall::Create(_src->src());
      } else {
        /// actually an identifier
        node = Identifier::Create(_src->src(), token->get_value());
      }
    } else if (token->get_type() == TokenType::PUNCTUATION && token->get_value() == "(") {
      node = Parenthesis::Create(_src->src());
    } else if (token->get_type() == TokenType::KEYWORD) { /// keywords
      node = peek_keyword(token);
      if (!node) {
        error(ErrorType::NOT_IMPLEMENTED, _curr, _curr, "Keyword not implemented: " + token->get_value());
      }
    } else if (token->get_type() == TokenType::BOP && token->get_value() == ".") { /// member access
      node = MemberAccess::Create(_src->src());
    } else if (token->get_value() == "&") {
      /// BOP or UOP? ambiguous
      node = BinaryOrUnary::Create(_src->src(), BinaryOperator::BOPPrecedence[BinaryOpKind::BAND]);
    } else if (token->get_type() == TokenType::PUNCTUATION && token->get_value() == "{") { /// statement(s)
      node = CompoundStmt::Create(_src->src());
    } else if (token->get_type() == TokenType::BOP) { /// binary operators that haven't been processed yet
      TAN_ASSERT(token->get_value().length());
      switch (token->get_value()[0]) {
      case '/':
        node = BinaryOperator::Create(BinaryOpKind::DIVIDE, _src->src());
        break;
      case '%':
        node = BinaryOperator::Create(BinaryOpKind::MOD, _src->src());
        break;
      case '|':
        node = BinaryOperator::Create(BinaryOpKind::BOR, _src->src());
        break;
      case '^':
        node = BinaryOperator::Create(BinaryOpKind::XOR, _src->src());
        break;
        /// Operators that are possibly BOP or UOP at this stage
        /// NOTE: using the precedence of the BOP form so that the parsing works correctly if it's really a BOP
      case '*':
        // MULTIPLY / PTR_DEREF
        node = BinaryOrUnary::Create(_src->src(), BinaryOperator::BOPPrecedence[BinaryOpKind::MULTIPLY]);
        break;
      case '+':
        // SUM / PLUS
        node = BinaryOrUnary::Create(_src->src(), BinaryOperator::BOPPrecedence[BinaryOpKind::SUM]);
        break;
      case '-':
        // SUBTRACT / MINUS
        node = BinaryOrUnary::Create(_src->src(), BinaryOperator::BOPPrecedence[BinaryOpKind::SUBTRACT]);
        break;
      default:
        TAN_ASSERT(false);
        return nullptr;
      }
    } else if (check_terminal_token(token)) { /// this MUST be the last thing to check
      return nullptr;
    } else {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Unknown token " + token->get_value());
    }

    node->set_start(_curr);
    node->set_end(_curr);
    return node;
  }

  ASTBase *next_expression(int rbp) {
    ASTBase *node = peek();
    if (!node) {
      return nullptr;
    }
    auto n = node;
    parse_node(n);
    auto left = n;
    node = peek();
    if (!node) {
      return left;
    }
    while (rbp < node->get_bp()) {
      node = peek();
      n = node;
      parse_node(left, n);
      left = n;
      node = peek();
      if (!node) {
        break;
      }
    }
    return left;
  }

  /// Parse NUD
  void parse_node(ASTBase *p) {
    /// special tokens that require whether p is led or nud to determine the node type
    if (p->get_node_type() == ASTNodeType::BOP_OR_UOP) {
      auto *pp = pcast<BinaryOrUnary>(p);
      UnaryOperator *actual = nullptr;
      str tok = _src->get_token_str(p->start());
      TAN_ASSERT(tok.length());
      switch (tok[0]) {
      case '*':
        actual = UnaryOperator::Create(UnaryOpKind::PTR_DEREF, _src->src());
        break;
      case '&':
        actual = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF, _src->src());
        break;
      case '+':
        actual = UnaryOperator::Create(UnaryOpKind::PLUS, _src->src());
        break;
      case '-':
        actual = UnaryOperator::Create(UnaryOpKind::MINUS, _src->src());
        break;
      default:
        TAN_ASSERT(false);
        break;
      }
      pp->set_uop(actual);

      parse_node(pp->get_expr_ptr());

      pp->set_end(pp->get_expr_ptr()->end());
      return;
    }

    // look up parser func from the table
    auto it = NUD_PARSING_FUNC_TABLE.find(p->get_node_type());
    if (it == NUD_PARSING_FUNC_TABLE.end()) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr,
            fmt::format("Unexpected token with type: {}", ASTBase::ASTTypeNames[p->get_node_type()]));
    }
    nud_parsing_func_t func = it->second;
    (this->*func)(p);
  }

  /// Parse LED
  void parse_node(ASTBase *left, ASTBase *p) {
    /// special tokens that require whether p is led or nud to determine the node type
    if (p->get_node_type() == ASTNodeType::BOP_OR_UOP) {
      auto *pp = pcast<BinaryOrUnary>(p);
      BinaryOperator *actual = nullptr;
      str tok = _src->get_token_str(p->start());
      TAN_ASSERT(tok.length());
      switch (tok[0]) {
      case '*':
        actual = BinaryOperator::Create(BinaryOpKind::MULTIPLY, _src->src());
        break;
      case '&':
        actual = BinaryOperator::Create(BinaryOpKind::BAND, _src->src());
        break;
      case '+':
        actual = BinaryOperator::Create(BinaryOpKind::SUM, _src->src());
        break;
      case '-':
        actual = BinaryOperator::Create(BinaryOpKind::SUBTRACT, _src->src());
        break;
      default:
        TAN_ASSERT(false);
        break;
      }
      pp->set_bop(actual);
      parse_node(left, pp->get_expr_ptr());

      pp->set_start(pp->get_expr_ptr()->start());
      pp->set_end(pp->get_expr_ptr()->end());
      return;
    }

    // look up parser func from the table
    auto it = LED_PARSING_FUNC_TABLE.find(p->get_node_type());
    if (it == LED_PARSING_FUNC_TABLE.end()) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr,
            fmt::format("Unexpected token with type: {}", ASTBase::ASTTypeNames[p->get_node_type()]));
    }
    led_parsing_func_t func = it->second;
    (this->*func)(left, p);
  }

private:
  [[nodiscard]] Token *at(uint32_t loc) const {
    if (this->eof(loc)) {
      Error(ErrorType::SYNTAX_ERROR, _src->get_last_token(), _src->get_last_token(), "Unexpected EOF").raise();
    }
    return _src->get_token(loc);
  }

  [[nodiscard]] bool eof(uint32_t loc) const { return _src->is_eof(loc); }

  [[noreturn]] void error(ErrorType type, ASTBase *node, const str &error_message) const {
    Error(type, at(node->start()), at(node->end()), error_message).raise();
  }

  [[noreturn]] void error(ErrorType type, uint32_t start, uint32_t end, const str &error_message) const {
    Error(type, at(start), at(end), error_message).raise();
  }

  Expr *expect_expression(ASTBase *p) {
    TAN_ASSERT(p);
    if (!p->is_expr())
      error(ErrorType::SYNTAX_ERROR, p, "Expect an expression");

    return pcast<Expr>(p);
  }

  Stmt *expect_stmt(ASTBase *p) {
    TAN_ASSERT(p);
    if (!p->is_stmt())
      error(ErrorType::SYNTAX_ERROR, p, "Expect a statement");

    return pcast<Stmt>(p);
  }

  void expect_token(const str &value) {
    Token *token = at(_curr);
    if (token->get_value() != value) {
      Error(ErrorType::SYNTAX_ERROR, token, token,
            fmt::format("Expect '{}' but got '{}' instead", value, token->get_value()))
          .raise();
    }
  }

private:
  void parse_compound_stmt(ASTBase *_p) {
    auto p = pcast<CompoundStmt>(_p);
    ScopeGuard scope_guard(_curr_scope, p);

    ++_curr; // skip "{"

    ASTBase *node = nullptr;
    while ((node = next_expression(PREC_LOWEST)) != nullptr) {
      p->append_child(node);

      str s = at(_curr)->get_value();
      expect_token(node->terminal_token());
      ++_curr;
    }

    expect_token("}");
    p->set_end(_curr);
  }

  void parse_assignment(ASTBase *left, ASTBase *_p) {
    auto p = pcast<Assignment>(_p);

    ++_curr; // skip =

    /// lhs
    p->set_lhs(left);

    /// rhs
    auto rhs = next_expression(PREC_LOWEST);
    p->set_rhs(expect_expression(rhs));
    p->set_end(_curr - 1);
  }

  void parse_cast(ASTBase *left, ASTBase *_p) {
    auto lhs = pcast<Expr>(left);
    auto p = pcast<Cast>(_p);

    ++_curr; // skip as

    /// lhs
    p->set_lhs(lhs);

    /// rhs
    auto *ty = peek_type();
    p->set_type(parse_ty(ty));

    p->set_end(_curr - 1);
  }

  void parse_generic_token(ASTBase *p) { p->set_end(_curr++); }

  void parse_if(ASTBase *_p) {
    auto p = pcast<If>(_p);

    p->set_end(_curr); // the end of token of If AST ends here

    /// if then
    parse_if_then_branch(p);
    ++_curr; // skip "}"

    /// else or elif clause, if any
    while (at(_curr)->get_value() == "else") {
      ++_curr;                              // skip "else"
      if (at(_curr)->get_value() == "if") { /// elif
        parse_if_then_branch(p);
      } else if (at(_curr)->get_value() == "{") { /// else
        auto else_clause = peek();
        parse_node(else_clause);
        p->add_else_branch(expect_stmt(else_clause));
      } else {
        error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Unexpected token");
      }

      ++_curr;
    }

    --_curr; /// return back to "}"
    TAN_ASSERT(at(_curr)->get_value() == "}");
  }

  void parse_if_then_branch(If *p) {
    ++_curr; // skip "if"

    // predicate
    expect_token("(");
    auto *_pred = peek();
    if (_pred->get_node_type() != ASTNodeType::PARENTHESIS) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Expect a parenthesis expression");
    }
    parse_parenthesis(_pred);
    Expr *pred = pcast<Expr>(_pred);

    // then clause
    expect_token("{");
    auto *_then = peek();
    parse_node(_then);
    Stmt *then_clause = expect_stmt(_then);

    p->add_if_then_branch(pred, then_clause);
  }

  void parse_loop(ASTBase *_p) {
    auto p = pcast<Loop>(_p);

    if (at(_curr)->get_value() == "for") {
      // TODO: implement for loop
      p->_loop_type = ASTLoopType::FOR;
    } else if (at(_curr)->get_value() == "while") {
      p->_loop_type = ASTLoopType::WHILE;
    } else {
      TAN_ASSERT(false);
    }

    p->set_end(_curr);
    ++_curr; // skip while/for

    switch (p->_loop_type) {
    case ASTLoopType::WHILE: {
      /// predicate
      expect_token("(");
      auto _pred = next_expression(PREC_LOWEST);
      Expr *pred = expect_expression(_pred);
      p->set_predicate(pred);
      expect_token("{");

      /// loop body
      auto _body = next_expression(PREC_LOWEST);
      Stmt *body = expect_stmt(_body);
      p->set_body(body);
      break;
    }
    case ASTLoopType::FOR:
      // TODO: implement for loop
      TAN_ASSERT(false);
    }
  }

  void parse_array_literal(ASTBase *_p) {
    auto *p = pcast<ArrayLiteral>(_p);

    ++_curr; // skip '['

    if (at(_curr)->get_value() == "]") {
      // TODO: support empty array literal, but raise error if the type cannot be inferred
      error(ErrorType::SEMANTIC_ERROR, p->start(), _curr, "Empty array literal");
    }

    vector<Literal *> elements{};
    while (!eof(_curr)) {
      if (at(_curr)->get_value() == ",") { /// skip ","
        ++_curr;
        continue;
      } else if (at(_curr)->get_value() == "]") { /// skip "]"
        ++_curr;
        break;
      }

      auto *node = peek();
      auto *expr = expect_expression(node);
      if (!expr->is_comptime_known()) {
        error(ErrorType::SEMANTIC_ERROR, _curr, _curr, "Expected a compile-time known value");
      }

      parse_node(node);
      elements.push_back(pcast<Literal>(node));
    }

    p->set_elements(elements);
    p->set_end(_curr - 1);
  }

  void parse_bop(ASTBase *_lhs, ASTBase *_p) {
    Expr *lhs = pcast<Expr>(_lhs);

    Token *token = at(_p->start());
    if (token->get_value() == "." || token->get_value() == "[") { /// delegate to parse_member_access
      parse_member_access(lhs, pcast<MemberAccess>(_p));
      return;
    }

    auto *p = pcast<BinaryOperator>(_p);

    ++_curr; // skip the operator

    p->set_lhs(lhs);

    auto rhs = next_expression(p->get_bp());
    p->set_rhs(expect_expression(rhs));

    p->set_start(lhs->start());
    p->set_end(_curr - 1);
  }

  void parse_uop(ASTBase *_p) {
    auto *p = pcast<UnaryOperator>(_p);

    ++_curr;
    auto rhs = pcast<Expr>(next_expression(p->get_bp()));
    if (!rhs) {
      error(ErrorType::SEMANTIC_ERROR, rhs, "Invalid operand");
    }
    p->set_rhs(rhs);
    p->set_end(_curr - 1);
  }

  void parse_parenthesis(ASTBase *_p) {
    auto *p = pcast<Parenthesis>(_p);

    ++_curr; // skip "("
    while (true) {
      auto *t = at(_curr);
      if (t->get_type() == TokenType::PUNCTUATION && t->get_value() == ")") { /// end at )
        ++_curr;
        break;
      }

      // NOTE: parenthesis without child expression inside are illegal
      // (except function call, which is parsed elsewhere)
      auto _sub = next_expression(PREC_LOWEST);
      Expr *sub = expect_expression(_sub);
      p->set_sub(sub);
    }

    p->set_end(_curr - 1);
  }

  void parse_func_decl(ASTBase *_p) {
    auto *p = pcast<FunctionDecl>(_p);

    bool is_public = false;
    bool is_external = false;
    str token_str = at(_curr)->get_value();
    if (token_str == "fn") { /// "fn"
      ++_curr;
    } else if (token_str == "pub") { /// "pub fn"
      is_public = true;
      _curr += 2;
    } else if (token_str == "extern") { /// "extern"
      is_external = true;
      _curr += 2;
    } else {
      TAN_ASSERT(false);
    }
    p->set_public(is_public);
    p->set_external(is_external);

    /// function name
    // Don't use peek since it look ahead and returns ASTNodeType::FUNCTION when it finds "(",
    // but we only want the function name as an identifier
    // [X] auto id = peek();
    Token *id_token = at(_curr);
    auto id = Identifier::Create(_src->src(), id_token->get_value());
    id->set_start(_curr);
    id->set_end(_curr);
    if (id->get_node_type() != ASTNodeType::ID) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Expect a function name");
    }
    parse_node(id);
    p->set_name(id->get_name());

    expect_token("(");
    ++_curr;

    // Register in the parent context
    if (is_public) {
      p->ctx()->set_function_decl(p);
    } else {
      p->ctx()->set_function_decl(p);
    }

    { // declarations of func arguments and variables in the body are within the local scope
      ScopeGuard scope_guard(_curr_scope, p);

      /// arguments
      vector<str> arg_names{};
      vector<Type *> arg_types{};
      vector<ArgDecl *> arg_decls{};
      if (at(_curr)->get_value() != ")") {
        while (!eof(_curr)) {
          auto arg = ArgDecl::Create(_src->src());
          arg->set_start(_curr);
          parse_node(arg);

          arg_names.push_back(arg->get_name());
          arg_types.push_back(arg->get_type());
          arg_decls.push_back(arg);

          if (at(_curr)->get_value() == ",") {
            ++_curr;
          } else {
            break;
          }
        }
      }

      expect_token(")");
      ++_curr;

      p->set_arg_names(arg_names);
      p->set_arg_decls(arg_decls);

      expect_token(":");
      ++_curr;

      /// function type
      auto *ret_type = peek_type();
      auto *func_type = Type::GetFunctionType(parse_ty(ret_type), arg_types);
      p->set_type(func_type);

      p->set_end(_curr - 1);

      /// body
      if (!is_external) {
        expect_token("{");
        auto body = peek();
        parse_node(body);
        p->set_body(expect_stmt(body));
      }
    }
  }

  void parse_func_call(ASTBase *_p) {
    auto *p = pcast<FunctionCall>(_p);

    p->set_name(at(_curr)->get_value()); // function name
    ++_curr;

    // No need to check since '(' is what distinguish a function call from an identifier at the first place
    // auto *token = at(_curr); if (token->get_value() != "(") { error("Invalid function call"); }
    ++_curr; // skip (

    // args
    while (!eof(_curr) && at(_curr)->get_value() != ")") {
      auto _arg = next_expression(PREC_LOWEST);
      Expr *arg = expect_expression(_arg);
      p->_args.push_back(arg);

      if (at(_curr)->get_value() == ",") { /// skip ,
        ++_curr;
      } else {
        break;
      }
    }

    expect_token(")");
    p->set_end(_curr);
    ++_curr;
  }

  // assuming _curr is at the token after "@"
  void parse_test_comp_error_intrinsic(Intrinsic *p) {
    ++_curr; // skip "test_comp_error"

    auto *e = peek();
    if (e->get_node_type() != ASTNodeType::PARENTHESIS) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Expect a parenthesis");
    }

    parse_node(e);
    auto *test_name = pcast<Parenthesis>(e)->get_sub();

    p->set_end(_curr - 1);

    if (test_name->get_node_type() != ASTNodeType::ID) {
      error(ErrorType::SEMANTIC_ERROR, test_name, "Expect a test name");
    }

    p->set_name("test_comp_error");
    p->set_intrinsic_type(IntrinsicType::TEST_COMP_ERROR);

    expect_token("{");
    auto *body = peek();
    try {
      parse_node(body);
      p->set_sub(body);
    } catch (const CompileException &e) {
      std::cerr << fmt::format("Caught expected compile error: {}\nContinue compilation...\n", e.what());
      p->set_sub(nullptr); // no need to check again in later stages

      while (at(_curr)->get_value() != "}")
        ++_curr;
    }
  }

  void parse_intrinsic(ASTBase *_p) {
    auto *p = pcast<Intrinsic>(_p);

    ++_curr; // skip "@"

    if (_src->get_token_str(_curr) == Intrinsic::TEST_COMP_ERROR_NAME) {
      parse_test_comp_error_intrinsic(p);
      return;
    }

    auto e = peek();
    parse_node(e);
    // Only allow identifier or function call as the valid intrinsic token
    if (e->get_node_type() != ASTNodeType::ID && e->get_node_type() != ASTNodeType::FUNC_CALL) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Unexpected token");
    }
    p->set_sub(e);

    // find out the name
    str name;
    switch (e->get_node_type()) {
    case ASTNodeType::FUNC_CALL:
      name = pcast<FunctionCall>(e)->get_name();
      break;
    case ASTNodeType::ID:
      name = pcast<Identifier>(e)->get_name();
      break;
    default:
      TAN_ASSERT(false);
      break;
    }
    TAN_ASSERT(!name.empty());

    // figure out which intrinsic
    p->set_name(name);
    auto q = Intrinsic::intrinsics.find(name);
    if (q == Intrinsic::intrinsics.end()) {
      error(ErrorType::UNKNOWN_SYMBOL, _curr, _curr, fmt::format("Unknown intrinsic {}", name));
    }
    p->set_intrinsic_type(q->second);

    // check if the syntax is correct
    switch (p->get_intrinsic_type()) {
    case IntrinsicType::LINENO:
    case IntrinsicType::FILENAME:
      if (e->get_node_type() != ASTNodeType::ID)
        error(ErrorType::SEMANTIC_ERROR, _curr, _curr, "Invalid usage of intrinsic");
      break;
    case IntrinsicType::NOOP:
    case IntrinsicType::STACK_TRACE:
    case IntrinsicType::TEST_COMP_ERROR:
      if (e->get_node_type() != ASTNodeType::FUNC_CALL)
        error(ErrorType::SEMANTIC_ERROR, _curr, _curr, "Invalid usage of intrinsic");
      break;
    case IntrinsicType::INVALID:
      TAN_ASSERT(false);
      break;
    default:
      break;
    }

    p->set_end(_curr - 1);
  }

  void parse_import(ASTBase *_p) {
    auto *p = pcast<Import>(_p);

    ++_curr; // skip "import"
    p->set_end(_curr);

    auto rhs = peek();
    if (rhs->get_node_type() != ASTNodeType::STRING_LITERAL) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Invalid import statement");
    }
    parse_node(rhs);
    str name = pcast<StringLiteral>(rhs)->get_value();
    p->set_name(name);
  }

  void parse_package_stmt(ASTBase *_p) {
    auto *p = pcast<PackageDecl>(_p);
    ++_curr;

    auto rhs = peek();
    if (rhs->get_node_type() != ASTNodeType::STRING_LITERAL) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Invalid package statement");
    }
    parse_node(rhs);
    str name = pcast<StringLiteral>(rhs)->get_value();

    p->set_name(name);

    p->set_end(_curr - 1);
  }

  void parse_member_access(Expr *left, MemberAccess *p) {
    if (at(_curr)->get_value() == "[") {
      p->_access_type = MemberAccess::MemberAccessBracket;
    }

    ++_curr; // skip "." or "["

    // lhs
    p->set_lhs(left);

    // rhs
    auto _right = peek();
    Expr *right = expect_expression(_right);
    parse_node(right);
    p->set_rhs(right);

    if (p->_access_type == MemberAccess::MemberAccessBracket) { // bracket access
      ++_curr;                                                  // skip ]
    }

    p->set_end(_curr - 1);
  }

  void parse_return(ASTBase *_p) {
    auto *p = pcast<Return>(_p);

    ++_curr; // skip "return"

    auto _rhs = next_expression(PREC_LOWEST);
    if (_rhs) {
      Expr *rhs = expect_expression(_rhs);
      p->set_rhs(rhs);
    }

    p->set_end(_curr - 1);
  }

  void parse_struct_decl(ASTBase *_p) {
    auto *p = pcast<StructDecl>(_p);

    ++_curr; // skip "struct"

    // struct typename
    auto _id = peek();
    if (_id->get_node_type() != ASTNodeType::ID) {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Expecting a typename");
    }
    parse_node(_id);
    auto id = pcast<Identifier>(_id);
    p->set_name(id->get_name());

    p->set_end(_curr - 1);

    // struct body
    if (at(_curr)->get_value() != "{") {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Expect struct body");
    }

    ScopeGuard scope_guard(_curr_scope, p);

    auto _comp_stmt = next_expression(PREC_LOWEST);
    if (!_comp_stmt || _comp_stmt->get_node_type() != ASTNodeType::COMPOUND_STATEMENT) {
      error(ErrorType::SEMANTIC_ERROR, _curr, _curr, "struct definition requires a valid body");
    }
    auto comp_stmt = pcast<CompoundStmt>(_comp_stmt);

    // copy member declarations
    auto children = comp_stmt->get_children();
    vector<Expr *> member_decls{};
    for (const auto &c : children) {
      if (!(                                                  //
              c->get_node_type() == ASTNodeType::VAR_DECL     //
              || c->get_node_type() == ASTNodeType::ASSIGN    //
              || c->get_node_type() == ASTNodeType::FUNC_DECL //
              )) {
        error(ErrorType::SEMANTIC_ERROR, c, "Invalid struct member");
      }
      member_decls.push_back(pcast<Expr>(c));
    }
    p->set_member_decls(member_decls);
  }

  ArrayType *parse_ty_array(Type *p) {
    ArrayType *ret = nullptr;
    while (true) {
      ++_curr; // skip "["

      // size
      ASTBase *_size = peek();
      if (_size->get_node_type() != ASTNodeType::INTEGER_LITERAL) {
        error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Expect an unsigned integer as the array size");
      }
      parse_node(_size);

      auto size = pcast<IntegerLiteral>(_size);
      size_t array_size = size->get_value();
      if (static_cast<int64_t>(array_size) < 0) {
        error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Expect an unsigned integer as the array size");
      }

      ret = Type::GetArrayType(p, (int)array_size);

      // skip "]"
      expect_token("]");
      ++_curr;

      // if followed by a "[", this is a multi-dimension array
      if (at(_curr)->get_value() == "[") {
        p = ret;
        ret = nullptr;
      } else {
        break;
      }
    }

    TAN_ASSERT(ret);
    return ret;
  }

  Type *parse_ty(Type *p) {
    Type *ret = p;

    while (!eof(_curr)) {
      Token *token = at(_curr);

      auto it = PrimitiveType::TYPENAME_TO_KIND.find(token->get_value());
      if (it != PrimitiveType::TYPENAME_TO_KIND.end()) { // primitive
        ret = PrimitiveType::Create(it->second);
      } else if (token->get_value() == "*") { // pointer
        TAN_ASSERT(ret);
        ret = Type::GetPointerType(ret);
      } else if (token->get_value() == "str") {
        ret = Type::GetStringType();
      } else if (token->get_type() == TokenType::ID) { // struct/typedefs etc.
        /// type referred will be resolved in analysis phase
      } else {
        break;
      }
      ++_curr;
    }

    // array
    Token *token = at(_curr);
    if (token->get_value() == "[") {
      TAN_ASSERT(ret);
      ret = parse_ty_array(ret);
    }

    return ret;
  }

  void parse_var_decl(ASTBase *_p) {
    auto *p = pcast<VarDecl>(_p);

    ++_curr; // skip 'var'

    // name
    auto name_token = at(_curr);
    p->set_name(name_token->get_value());
    ++_curr;

    // type
    if (at(_curr)->get_value() == ":") {
      ++_curr;
      Type *ty = peek_type();
      p->set_type(parse_ty(ty));
    }

    p->set_end(_curr - 1);
  }

  void parse_arg_decl(ASTBase *_p) {
    auto *p = pcast<ArgDecl>(_p);

    // name
    auto name_token = at(_curr);
    p->set_name(name_token->get_value());
    ++_curr;

    if (at(_curr)->get_value() != ":") {
      error(ErrorType::SYNTAX_ERROR, _curr, _curr, "Expect a type being specified");
    }
    ++_curr;

    // type
    Type *ty = peek_type();
    p->set_type(parse_ty(ty));

    p->set_end(_curr - 1);
  }

private:
  TokenizedSourceFile *_src = nullptr;
  uint32_t _curr = 0;
  str _filename;
  Program *_root = nullptr;
  ASTBase *_curr_scope = nullptr;

private:
  const static umap<ASTNodeType, nud_parsing_func_t> NUD_PARSING_FUNC_TABLE;
  const static umap<ASTNodeType, led_parsing_func_t> LED_PARSING_FUNC_TABLE;
};

Parser::Parser(TokenizedSourceFile *src) { _impl = new ParserImpl(src); }

Program *Parser::parse() { return _impl->parse(); }

Parser::~Parser() { delete _impl; }

const umap<ASTNodeType, nud_parsing_func_t> ParserImpl::NUD_PARSING_FUNC_TABLE = {
    {ASTNodeType::COMPOUND_STATEMENT, &ParserImpl::parse_compound_stmt},
    {ASTNodeType::PARENTHESIS,        &ParserImpl::parse_parenthesis  },
    {ASTNodeType::IMPORT,             &ParserImpl::parse_import       },
    {ASTNodeType::INTRINSIC,          &ParserImpl::parse_intrinsic    },
    {ASTNodeType::IF,                 &ParserImpl::parse_if           },
    {ASTNodeType::LOOP,               &ParserImpl::parse_loop         },
    {ASTNodeType::UOP,                &ParserImpl::parse_uop          },
    {ASTNodeType::RET,                &ParserImpl::parse_return       },
    {ASTNodeType::FUNC_CALL,          &ParserImpl::parse_func_call    },
    {ASTNodeType::ARRAY_LITERAL,      &ParserImpl::parse_array_literal},
    {ASTNodeType::STRUCT_DECL,        &ParserImpl::parse_struct_decl  },
    {ASTNodeType::VAR_DECL,           &ParserImpl::parse_var_decl     },
    {ASTNodeType::ARG_DECL,           &ParserImpl::parse_arg_decl     },
    {ASTNodeType::FUNC_DECL,          &ParserImpl::parse_func_decl    },
    {ASTNodeType::BREAK,              &ParserImpl::parse_generic_token},
    {ASTNodeType::CONTINUE,           &ParserImpl::parse_generic_token},
    {ASTNodeType::ID,                 &ParserImpl::parse_generic_token},
    {ASTNodeType::INTEGER_LITERAL,    &ParserImpl::parse_generic_token},
    {ASTNodeType::FLOAT_LITERAL,      &ParserImpl::parse_generic_token},
    {ASTNodeType::CHAR_LITERAL,       &ParserImpl::parse_generic_token},
    {ASTNodeType::STRING_LITERAL,     &ParserImpl::parse_generic_token},
    {ASTNodeType::BOOL_LITERAL,       &ParserImpl::parse_generic_token},
    {ASTNodeType::PACKAGE_DECL,       &ParserImpl::parse_package_stmt },
};
} // namespace tanlang

const umap<ASTNodeType, led_parsing_func_t> ParserImpl::LED_PARSING_FUNC_TABLE = {
    {ASTNodeType::BOP,    &ParserImpl::parse_bop       },
    {ASTNodeType::ASSIGN, &ParserImpl::parse_assignment},
    {ASTNodeType::CAST,   &ParserImpl::parse_cast      }
};

// implementations of helper functions

static bool check_typename_token(Token *token) {
  return is_string_in(token->get_value(), tanlang::Type::ALL_TYPE_NAMES);
}

static bool check_terminal_token(Token *token) {
  // return token->get_type() == TokenType::PUNCTUATION && is_string_in(token->get_value(), TERMINAL_TOKENS);
  return is_string_in(token->get_value(), TERMINAL_TOKENS);
}
