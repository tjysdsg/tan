#include "src/ast/ast_node_type.h"
#include "src/ast/ast_base.h"
#include <iostream>

using namespace tanlang;

ASTBase::ASTBase(ASTNodeType node_type, SourceIndex loc, int bp)
    : SourceTraceable(loc), _node_type(node_type), _bp(bp) {}

ASTNodeType ASTBase::get_node_type() const { return _node_type; }

void ASTBase::set_node_type(ASTNodeType node_type) {
  _node_type = node_type;
}

void ASTBase::set_bp(int bp) { _bp = bp; }

int ASTBase::get_bp() const { return _bp; }

void ASTBase::printTree() const {
  using std::cout;
  cout << this->to_string(true) << "\n";
  vector<ASTBase *> children = get_children();
  size_t n_children = children.size();
  for (size_t i = 0; i < n_children; ++i) {
    auto *ch = children[i];
    if (ch) {
      ch->printTree("", i >= n_children - 1);
    }
  }
}

void ASTBase::printTree(const str &prefix, bool last_child) const {
  using std::cout;
  vector<ASTBase *> children = get_children();
  cout << prefix << (last_child ? "└── " : "├── ") << this->to_string(true) << "\n";
  if (children.empty()) { return; }
  size_t n_children = children.size();
  for (size_t i = 0; i < n_children; ++i) {
    auto *c = children[i];
    if (c) {
      c->printTree(prefix + (last_child ? "     " : "│    "), i >= n_children - 1);
    }
  }
}

str ASTBase::to_string(bool print_prefix) const {
  if (print_prefix) { return ASTTypeNames[this->_node_type]; }
  else { return ""; }
}

ASTBase *ASTBase::get() const { return const_cast<ASTBase *>(this); }

vector<ASTBase *> ASTBase::get_children() const { return {}; }

#define MAKE_ASTTYPE_NAME_PAIR(t) {ASTNodeType::t, #t}

umap<ASTNodeType, str>ASTBase::ASTTypeNames =
    {MAKE_ASTTYPE_NAME_PAIR(PROGRAM), MAKE_ASTTYPE_NAME_PAIR(STATEMENT), MAKE_ASTTYPE_NAME_PAIR(STRING_LITERAL),
        MAKE_ASTTYPE_NAME_PAIR(RET), MAKE_ASTTYPE_NAME_PAIR(IF), MAKE_ASTTYPE_NAME_PAIR(ID),
        MAKE_ASTTYPE_NAME_PAIR(PARENTHESIS), MAKE_ASTTYPE_NAME_PAIR(FUNC_CALL), MAKE_ASTTYPE_NAME_PAIR(FUNC_DECL),
        MAKE_ASTTYPE_NAME_PAIR(ARG_DECL), MAKE_ASTTYPE_NAME_PAIR(VAR_DECL), MAKE_ASTTYPE_NAME_PAIR(TY),
        MAKE_ASTTYPE_NAME_PAIR(ARRAY_LITERAL), MAKE_ASTTYPE_NAME_PAIR(INTRINSIC), MAKE_ASTTYPE_NAME_PAIR(LOOP),
        MAKE_ASTTYPE_NAME_PAIR(IMPORT), MAKE_ASTTYPE_NAME_PAIR(CHAR_LITERAL), MAKE_ASTTYPE_NAME_PAIR(BREAK),
        MAKE_ASTTYPE_NAME_PAIR(CONTINUE), MAKE_ASTTYPE_NAME_PAIR(ENUM_DECL), MAKE_ASTTYPE_NAME_PAIR(CAST),
        MAKE_ASTTYPE_NAME_PAIR(ASSIGN), MAKE_ASTTYPE_NAME_PAIR(VAR_REF)};

#undef MAKE_ASTTYPE_NAME_PAIR

umap<ASTNodeType, int>ASTBase::OpPrecedence =
    {{ASTNodeType::PROGRAM, PREC_LOWEST}, {ASTNodeType::STATEMENT, PREC_LOWEST}, {ASTNodeType::INVALID, PREC_LOWEST},
        {ASTNodeType::PARENTHESIS, PREC_CALL}, {ASTNodeType::RET, PREC_LOWEST}, {ASTNodeType::IF, PREC_LOWEST},
        {ASTNodeType::STRING_LITERAL, PREC_LITERAL}, {ASTNodeType::CAST, PREC_CAST},
        {ASTNodeType::ASSIGN, PREC_ASSIGN}};
