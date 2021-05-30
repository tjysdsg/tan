#include "src/ast/ast_base.h"
#include "src/ast/ast_node_type.h"
#include <iostream>
#include "src/ast/ast_node.h"
#include "src/ast/ast_ty.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_member_access.h"
#include "src/ast/ast_control_flow.h"

using namespace tanlang;

ASTNodeType ASTBase::get_node_type() const {
  return _node_type;
}

void ASTBase::set_node_type(ASTNodeType node_type) {
  _node_type = node_type;
}

void ASTBase::set_lbp(int lbp) {
  _lbp = lbp;
}

int ASTBase::get_lbp() const {
  return _lbp;
}

/*
 * TODO
 *  void ASTBase::printTree() {
 *   using std::cout;
 *   cout << this->to_string(true) << "\n";
 *   size_t n_children = _children.size();
 *   for (size_t i = 0; i < n_children; ++i) {
 *     _children[i]->printTree("", i >= n_children - 1);
 *   }
 *  }
 *  void ASTBase::printTree(const str &prefix, bool last_child) {
 *    using std::cout;
 *    cout << prefix << (last_child ? "└── " : "├── ") << this->to_string(true) << "\n";
 *    if (_children.empty()) { return; }
 *    size_t n_children = _children.size();
 *    for (size_t i = 0; i < n_children; ++i) {
 *      const auto &c = _children[i];
 *      c->printTree(prefix + (last_child ? "     " : "│    "), i >= n_children - 1);
 *    }
 *  }
 */

str ASTBase::to_string(bool print_prefix) {
  if (print_prefix) { return ASTTypeNames[this->_node_type]; }
  else { return ""; }
}

void ASTBase::set_scope(const ptr<Scope> &scope) {
  _scope = scope;
}

ptr<Scope> ASTBase::get_scope() const {
  return _scope;
}

#define MAKE_ASTTYPE_NAME_PAIR(t) {ASTNodeType::t, #t}

umap<ASTNodeType, str>ASTBase::ASTTypeNames =
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

umap<ASTNodeType, int>ASTBase::OpPrecedence =
    {{ASTNodeType::PROGRAM, PREC_LOWEST}, {ASTNodeType::STATEMENT, PREC_LOWEST}, {ASTNodeType::INVALID, PREC_LOWEST},
        {ASTNodeType::SUM, PREC_TERM}, {ASTNodeType::SUBTRACT, PREC_TERM}, {ASTNodeType::BOR, PREC_TERM},
        {ASTNodeType::XOR, PREC_TERM}, {ASTNodeType::MULTIPLY, PREC_FACTOR}, {ASTNodeType::DIVIDE, PREC_FACTOR},
        {ASTNodeType::MOD, PREC_FACTOR}, {ASTNodeType::BAND, PREC_FACTOR}, {ASTNodeType::GT, PREC_COMPARISON},
        {ASTNodeType::GE, PREC_COMPARISON}, {ASTNodeType::NE, PREC_COMPARISON}, {ASTNodeType::LT, PREC_COMPARISON},
        {ASTNodeType::LE, PREC_COMPARISON}, {ASTNodeType::EQ, PREC_COMPARISON}, {ASTNodeType::ASSIGN, PREC_ASSIGN},
        {ASTNodeType::PARENTHESIS, PREC_CALL}, {ASTNodeType::MEMBER_ACCESS, PREC_HIGHEST},
        {ASTNodeType::RET, PREC_LOWEST}, {ASTNodeType::IF, PREC_LOWEST}, {ASTNodeType::ELSE, PREC_LOWEST},
        {ASTNodeType::BNOT, PREC_UNARY}, {ASTNodeType::LNOT, PREC_UNARY}, {ASTNodeType::LAND, PREC_LOGICAL_AND},
        {ASTNodeType::LOR, PREC_LOGICAL_OR}, {ASTNodeType::NUM_LITERAL, PREC_LITERAL},
        {ASTNodeType::STRING_LITERAL, PREC_LITERAL}, {ASTNodeType::CAST, PREC_CAST},
        {ASTNodeType::ADDRESS_OF, PREC_UNARY}};
