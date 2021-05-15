#include "src/ast/parsable_ast_node.h"
#include "src/ast/ast_type.h"
#include <iostream>

using namespace tanlang;

ptr<ParsableASTNode> ParsableASTNode::get_child_at(size_t idx) {
  TAN_ASSERT(_children.size() > idx);
  return _children[idx];
}

void ParsableASTNode::set_child_at(size_t idx, ptr<ParsableASTNode> node) {
  TAN_ASSERT(_children.size() > idx);
  _children[idx] = node;
}

void ParsableASTNode::append_child(ptr<ParsableASTNode> node) {
  _children.push_back(node);
}

void ParsableASTNode::clear_children() {
  _children.clear();
}

size_t ParsableASTNode::get_children_size() {
  return _children.size();
}

vector<ParsableASTNodePtr> &ParsableASTNode::get_children() {
  return _children;
}

vector<ParsableASTNodePtr> ParsableASTNode::get_children() const {
  return _children;
}

ASTType ParsableASTNode::get_node_type() {
  return _type;
}

void ParsableASTNode::set_node_type(ASTType node_type) {
  _type = node_type;
}

void ParsableASTNode::set_lbp(int lbp) {
  _lbp = lbp;
}

int ParsableASTNode::get_lbp() {
  return _lbp;
}

void ParsableASTNode::printTree() {
  using std::cout;
  cout << this->to_string(true) << "\n";
  size_t n_children = _children.size();
  for (size_t i = 0; i < n_children; ++i) {
    _children[i]->printTree("", i >= n_children - 1);
  }
}

void ParsableASTNode::printTree(const str &prefix, bool last_child) {
  using std::cout;

  cout << prefix << (last_child ? "└── " : "├── ") << this->to_string(true) << "\n";
  if (_children.empty()) { return; }
  size_t n_children = _children.size();

  for (size_t i = 0; i < n_children; ++i) {
    const auto &c = _children[i];
    c->printTree(prefix + (last_child ? "     " : "│    "), i >= n_children - 1);
  }
}

str ParsableASTNode::to_string(bool print_prefix) {
  if (print_prefix) { return ASTTypeNames[this->_type]; }
  else { return ""; }
}

#define MAKE_ASTTYPE_NAME_PAIR(t) {ASTType::t, #t}

umap<ASTType, str>ParsableASTNode::ASTTypeNames =
    {MAKE_ASTTYPE_NAME_PAIR(PROGRAM), MAKE_ASTTYPE_NAME_PAIR(STATEMENT), MAKE_ASTTYPE_NAME_PAIR(SUM),
        MAKE_ASTTYPE_NAME_PAIR(SUBTRACT), MAKE_ASTTYPE_NAME_PAIR(MULTIPLY), MAKE_ASTTYPE_NAME_PAIR(DIVIDE),
        MAKE_ASTTYPE_NAME_PAIR(MOD), MAKE_ASTTYPE_NAME_PAIR(ASSIGN), MAKE_ASTTYPE_NAME_PAIR(NUM_LITERAL),
        MAKE_ASTTYPE_NAME_PAIR(STRING_LITERAL), MAKE_ASTTYPE_NAME_PAIR(BAND), MAKE_ASTTYPE_NAME_PAIR(LAND),
        MAKE_ASTTYPE_NAME_PAIR(BOR), MAKE_ASTTYPE_NAME_PAIR(LOR), MAKE_ASTTYPE_NAME_PAIR(BNOT),
        MAKE_ASTTYPE_NAME_PAIR(LNOT), MAKE_ASTTYPE_NAME_PAIR(XOR), MAKE_ASTTYPE_NAME_PAIR(RET),
        MAKE_ASTTYPE_NAME_PAIR(IF), MAKE_ASTTYPE_NAME_PAIR(ELSE), MAKE_ASTTYPE_NAME_PAIR(GT),
        MAKE_ASTTYPE_NAME_PAIR(GE), MAKE_ASTTYPE_NAME_PAIR(LT), MAKE_ASTTYPE_NAME_PAIR(LE), MAKE_ASTTYPE_NAME_PAIR(ID),
        MAKE_ASTTYPE_NAME_PAIR(PARENTHESIS), MAKE_ASTTYPE_NAME_PAIR(FUNC_CALL), MAKE_ASTTYPE_NAME_PAIR(FUNC_DECL),
        MAKE_ASTTYPE_NAME_PAIR(ARG_DECL), MAKE_ASTTYPE_NAME_PAIR(VAR_DECL), MAKE_ASTTYPE_NAME_PAIR(TY),
        MAKE_ASTTYPE_NAME_PAIR(MEMBER_ACCESS), MAKE_ASTTYPE_NAME_PAIR(ARRAY_LITERAL), MAKE_ASTTYPE_NAME_PAIR(EQ),
        MAKE_ASTTYPE_NAME_PAIR(INTRINSIC), MAKE_ASTTYPE_NAME_PAIR(LOOP), MAKE_ASTTYPE_NAME_PAIR(NE),
        MAKE_ASTTYPE_NAME_PAIR(IMPORT), MAKE_ASTTYPE_NAME_PAIR(CAST), MAKE_ASTTYPE_NAME_PAIR(CHAR_LITERAL),
        MAKE_ASTTYPE_NAME_PAIR(ADDRESS_OF), MAKE_ASTTYPE_NAME_PAIR(BREAK), MAKE_ASTTYPE_NAME_PAIR(CONTINUE),
        MAKE_ASTTYPE_NAME_PAIR(ENUM_DECL)};

#undef MAKE_ASTTYPE_NAME_PAIR

umap<ASTType, int>ParsableASTNode::OpPrecedence =
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
