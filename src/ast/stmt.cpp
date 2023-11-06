#include "ast/stmt.h"
#include <algorithm>

using namespace tanlang;

/// \section Stmt

Stmt::Stmt(ASTNodeType type, TokenizedSourceFile *src) : ASTBase(type, src, PREC_LOWEST) {}

vector<ASTBase *> Stmt::get_children() const { return {}; }

/// \section Compound statement

CompoundStmt *CompoundStmt::Create(TokenizedSourceFile *src) { return new CompoundStmt(src); }

CompoundStmt::CompoundStmt(TokenizedSourceFile *src) : Stmt(ASTNodeType::COMPOUND_STATEMENT, src) {}

void CompoundStmt::set_child_at(size_t idx, ASTBase *node) {
  TAN_ASSERT(_children.size() > idx);
  _children[idx] = node;
}

void CompoundStmt::append_child(ASTBase *node) { _children.push_back(node); }

void CompoundStmt::clear_children() { _children.clear(); }

size_t CompoundStmt::get_children_size() const { return _children.size(); }

vector<ASTBase *> &CompoundStmt::get_children() { return _children; }

vector<ASTBase *> CompoundStmt::get_children() const { return _children; }

/// \section Program

Program *Program::Create(TokenizedSourceFile *src) { return new Program(src); }

Program::Program(TokenizedSourceFile *src) : CompoundStmt(src) { set_node_type((ASTNodeType::PROGRAM)); }

Return *Return::Create(TokenizedSourceFile *src) { return new Return(src); }

/// \section Return

Return::Return(TokenizedSourceFile *src) : Stmt(ASTNodeType::RET, src) {}

void Return::set_rhs(Expr *rhs) { _rhs = rhs; }

Expr *Return::get_rhs() const { return _rhs; }

vector<ASTBase *> Return::get_children() const { return {(ASTBase *)_rhs}; }

/// \section Import

Import *Import::Create(TokenizedSourceFile *src) { return new Import(src); }

Import::Import(TokenizedSourceFile *src) : Stmt(ASTNodeType::IMPORT, src) {}

void Import::set_name(const str &s) { _name = s; }

const str &Import::get_name() const { return _name; }

const vector<FunctionDecl *> &Import::get_imported_funcs() const { return _imported_funcs; }

void Import::set_imported_funcs(const vector<FunctionDecl *> &imported_funcs) { _imported_funcs = imported_funcs; }

/// \section Break or continue statement

BreakContinue::BreakContinue(ASTNodeType type, TokenizedSourceFile *src) : Stmt(type, src) {
  TAN_ASSERT(type == ASTNodeType::BREAK || type == ASTNodeType::CONTINUE);
}

Loop *BreakContinue::get_parent_loop() const { return _parent_loop; }

void BreakContinue::set_parent_loop(Loop *parent_loop) { _parent_loop = parent_loop; }

Break *Break::Create(TokenizedSourceFile *src) { return new Break(src); }

Break::Break(TokenizedSourceFile *src) : BreakContinue(ASTNodeType::BREAK, src) {}

Continue *Continue::Create(TokenizedSourceFile *src) { return new Continue(src); }

Continue::Continue(TokenizedSourceFile *src) : BreakContinue(ASTNodeType::CONTINUE, src) {}

/// \section Loop

Loop *Loop::Create(TokenizedSourceFile *src) { return new Loop(src); }

Loop::Loop(TokenizedSourceFile *src) : Stmt(ASTNodeType::LOOP, src) {}

void Loop::set_body(Stmt *body) { _body = body; }

void Loop::set_predicate(Expr *pred) { _predicate = pred; }

Expr *Loop::get_predicate() const { return _predicate; }

Stmt *Loop::get_body() const { return _body; }

vector<ASTBase *> Loop::get_children() const { return {(ASTBase *)_predicate, _body}; }

/// \section If-else

If::If(TokenizedSourceFile *src) : Stmt(ASTNodeType::IF, src) {}

If *If::Create(TokenizedSourceFile *src) { return new If(src); }

void If::add_if_then_branch(Expr *pred, Stmt *branch) {
  TAN_ASSERT(pred);
  _predicates.push_back(pred);
  _branches.push_back(branch);
}

void If::add_else_branch(Stmt *branch) {
  _predicates.push_back(nullptr);
  _branches.push_back(branch);
}

Expr *If::get_predicate(size_t i) const {
  TAN_ASSERT(i < _predicates.size());
  return _predicates[i];
}

void If::set_predicate(size_t i, Expr *expr) {
  TAN_ASSERT(i < _predicates.size());
  _predicates[i] = expr;
}

Stmt *If::get_branch(size_t i) const {
  TAN_ASSERT(i < _branches.size());
  return _branches[i];
}

size_t If::get_num_branches() const { return _branches.size(); }

vector<ASTBase *> If::get_children() const {
  vector<ASTBase *> ret = {};
  std::for_each(_predicates.begin(), _predicates.end(), [&](Expr *e) { ret.push_back((ASTBase *)e); });
  std::for_each(_branches.begin(), _branches.end(), [&](Stmt *e) { ret.push_back((ASTBase *)e); });
  return ret;
}

PackageDecl::PackageDecl(TokenizedSourceFile *src) : Stmt(ASTNodeType::PACKAGE_DECL, src) {}

PackageDecl *PackageDecl::Create(TokenizedSourceFile *src) { return new PackageDecl(src); }

str PackageDecl::get_name() const { return _name; }

void PackageDecl::set_name(const str &name) { _name = name; }
