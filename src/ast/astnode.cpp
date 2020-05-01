#include "src/llvm_include.h"
#include "src/ast/astnode.h"
#include "parser.h"

namespace tanlang {

std::string ASTNode::to_string(bool print_prefix) const {
  if (print_prefix) { return ast_type_names[this->_type]; }
  else { return ""; }
}

void ASTNode::printTree() const {
  using std::cout;
  cout << this->to_string(true) << "\n";
  size_t n_children = _children.size();
  for (size_t i = 0; i < n_children; ++i) {
    _children[i]->printTree("", i >= n_children - 1);
  }
}

void ASTNode::printTree(const std::string &prefix, bool last_child) const {
  using std::cout;

  cout << prefix << (last_child ? "└── " : "├── ") << this->to_string(true) << "\n";
  if (_children.empty()) { return; }
  size_t n_children = _children.size();

  for (size_t i = 0; i < n_children; ++i) {
    const auto &c = _children[i];
    c->printTree(prefix + (last_child ? "     " : "│    "), i >= n_children - 1);
  }
}

std::string ASTNode::get_src() const {
  std::string ret = "";
  for (size_t i = _start_index; i < _end_index; ++i) {
    ret += _parser->at(i)->value;
    if (i < _end_index - 1) {
      ret += " ";
    }
  }
  return ret;
}

ASTNode::ASTNode(ASTType op, int lbp, int rbp, Token *token, size_t token_index)
    : _type(op), _lbp(lbp), _rbp(rbp), _token(token), _start_index(token_index) {}

size_t ASTNode::parse(const ASTNodePtr &left, Parser *parser, CompilerSession *cs) {
  _parser = parser;
  _cs = cs;
  auto ret = this->led(left);
  _parsed = true;
  return ret;
}

size_t ASTNode::parse(Parser *parser, CompilerSession *cs) {
  _parser = parser;
  _cs = cs;
  auto ret = this->nud();
  _parsed = true;
  return ret;
}

size_t ASTNode::led(const ASTNodePtr &left) {
  UNUSED(left);
  throw std::runtime_error("Not implemented");
}

size_t ASTNode::nud() { throw std::runtime_error("Not implemented"); }

Value *ASTNode::codegen(CompilerSession *compiler_session) {
  if (_children.empty()) { return nullptr; }
  size_t n = _children.size();
  Value *result = nullptr;
  for (size_t i = 0; i < n - 1; ++i) {
    _children[i]->codegen(compiler_session);
  }
  result = _children[n - 1]->codegen(compiler_session);
  return result;
}

/// other definitions
#define MAKE_ASTTYPE_NAME_PAIR(t) {ASTType::t, #t}

std::unordered_map<ASTType, std::string> ast_type_names
    {MAKE_ASTTYPE_NAME_PAIR(PROGRAM), MAKE_ASTTYPE_NAME_PAIR(STATEMENT), MAKE_ASTTYPE_NAME_PAIR(SUM),
        MAKE_ASTTYPE_NAME_PAIR(SUBTRACT), MAKE_ASTTYPE_NAME_PAIR(MULTIPLY), MAKE_ASTTYPE_NAME_PAIR(DIVIDE),
        MAKE_ASTTYPE_NAME_PAIR(MOD), MAKE_ASTTYPE_NAME_PAIR(ASSIGN), MAKE_ASTTYPE_NAME_PAIR(STRUCT_DECL),
        MAKE_ASTTYPE_NAME_PAIR(NUM_LITERAL), MAKE_ASTTYPE_NAME_PAIR(STRING_LITERAL), MAKE_ASTTYPE_NAME_PAIR(BAND),
        MAKE_ASTTYPE_NAME_PAIR(LAND), MAKE_ASTTYPE_NAME_PAIR(BOR), MAKE_ASTTYPE_NAME_PAIR(LOR),
        MAKE_ASTTYPE_NAME_PAIR(BNOT), MAKE_ASTTYPE_NAME_PAIR(LNOT), MAKE_ASTTYPE_NAME_PAIR(XOR),
        MAKE_ASTTYPE_NAME_PAIR(RET), MAKE_ASTTYPE_NAME_PAIR(IF), MAKE_ASTTYPE_NAME_PAIR(ELSE),
        MAKE_ASTTYPE_NAME_PAIR(GT), MAKE_ASTTYPE_NAME_PAIR(GE), MAKE_ASTTYPE_NAME_PAIR(LT), MAKE_ASTTYPE_NAME_PAIR(LE),
        MAKE_ASTTYPE_NAME_PAIR(ID), MAKE_ASTTYPE_NAME_PAIR(PARENTHESIS), MAKE_ASTTYPE_NAME_PAIR(FUNC_CALL),
        MAKE_ASTTYPE_NAME_PAIR(FUNC_DECL), MAKE_ASTTYPE_NAME_PAIR(ARG_DECL), MAKE_ASTTYPE_NAME_PAIR(VAR_DECL),
        MAKE_ASTTYPE_NAME_PAIR(TY), MAKE_ASTTYPE_NAME_PAIR(MEMBER_ACCESS), MAKE_ASTTYPE_NAME_PAIR(ARRAY_LITERAL),
        MAKE_ASTTYPE_NAME_PAIR(EQ), MAKE_ASTTYPE_NAME_PAIR(INTRINSIC), MAKE_ASTTYPE_NAME_PAIR(LOOP),
        MAKE_ASTTYPE_NAME_PAIR(NE), MAKE_ASTTYPE_NAME_PAIR(IMPORT), MAKE_ASTTYPE_NAME_PAIR(CAST),
        MAKE_ASTTYPE_NAME_PAIR(CHAR_LITERAL), MAKE_ASTTYPE_NAME_PAIR(ADDRESS_OF)};

#undef MAKE_ASTTYPE_NAME_PAIR

/// operator precedence
std::unordered_map<ASTType, int> op_precedence
    {{ASTType::PROGRAM, PREC_LOWEST}, {ASTType::STATEMENT, PREC_LOWEST}, {ASTType::INVALID, PREC_LOWEST},
        {ASTType::SUM, PREC_TERM}, {ASTType::SUBTRACT, PREC_TERM}, {ASTType::BOR, PREC_TERM}, {ASTType::XOR, PREC_TERM},
        {ASTType::MULTIPLY, PREC_FACTOR}, {ASTType::DIVIDE, PREC_FACTOR}, {ASTType::MOD, PREC_FACTOR},
        {ASTType::BAND, PREC_FACTOR}, {ASTType::GT, PREC_COMPARISON}, {ASTType::GE, PREC_COMPARISON},
        {ASTType::NE, PREC_COMPARISON}, {ASTType::LT, PREC_COMPARISON}, {ASTType::LE, PREC_COMPARISON},
        {ASTType::EQ, PREC_COMPARISON}, {ASTType::ASSIGN, PREC_ASSIGN}, {ASTType::PARENTHESIS, PREC_CALL},
        {ASTType::MEMBER_ACCESS, PREC_HIGHEST}, {ASTType::RET, PREC_LOWEST}, {ASTType::IF, PREC_LOWEST},
        {ASTType::ELSE, PREC_LOWEST}, {ASTType::BNOT, PREC_UNARY}, {ASTType::LNOT, PREC_UNARY},
        {ASTType::LAND, PREC_LOGICAL_AND}, {ASTType::LOR, PREC_LOGICAL_OR}, {ASTType::NUM_LITERAL, PREC_LITERAL},
        {ASTType::STRING_LITERAL, PREC_LITERAL}, {ASTType::CAST, PREC_CAST}, {ASTType::ADDRESS_OF, PREC_UNARY}};

} // namespace tanlang
