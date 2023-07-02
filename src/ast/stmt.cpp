#include "ast/stmt.h"
#include <algorithm>

using namespace tanlang;

/// \section Stmt

Stmt::Stmt(ASTNodeType type, SourceFile *src) : ASTBase(type, src, PREC_LOWEST) {}

vector<ASTBase *> Stmt::get_children() const { return {}; }

/// \section Compound statement

CompoundStmt *CompoundStmt::Create(SourceFile *src) { return new CompoundStmt(src); }

CompoundStmt::CompoundStmt(SourceFile *src) : Stmt(ASTNodeType::COMPOUND_STATEMENT, src) {}

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

Program *Program::Create(SourceFile *src) { return new Program(src); }

Program::Program(SourceFile *src) : CompoundStmt(src) { set_node_type((ASTNodeType::PROGRAM)); }

Return *Return::Create(SourceFile *src) { return new Return(src); }

/// \section Return

Return::Return(SourceFile *src) : Stmt(ASTNodeType::RET, src) {}

void Return::set_rhs(Expr *rhs) { _rhs = rhs; }

Expr *Return::get_rhs() const { return _rhs; }

vector<ASTBase *> Return::get_children() const { return {(ASTBase *)_rhs}; }

/// \section Import

Import *Import::Create(SourceFile *src) { return new Import(src); }

Import::Import(SourceFile *src) : Stmt(ASTNodeType::IMPORT, src) {}

void Import::set_filename(const str &s) { _filename = s; }

const str &Import::get_filename() const { return _filename; }

const vector<FunctionDecl *> &Import::get_imported_funcs() const { return _imported_funcs; }

void Import::set_imported_funcs(const vector<FunctionDecl *> &imported_funcs) { _imported_funcs = imported_funcs; }

/// \section Break or continue statement

BreakContinue::BreakContinue(ASTNodeType type, SourceFile *src) : Stmt(type, src) {
  TAN_ASSERT(type == ASTNodeType::BREAK || type == ASTNodeType::CONTINUE);
}

Loop *BreakContinue::get_parent_loop() const { return _parent_loop; }

void BreakContinue::set_parent_loop(Loop *parent_loop) { _parent_loop = parent_loop; }

Break *Break::Create(SourceFile *src) { return new Break(src); }

Break::Break(SourceFile *src) : BreakContinue(ASTNodeType::BREAK, src) {}

Continue *Continue::Create(SourceFile *src) { return new Continue(src); }

Continue::Continue(SourceFile *src) : BreakContinue(ASTNodeType::CONTINUE, src) {}

/// \section Loop

Loop *Loop::Create(SourceFile *src) { return new Loop(src); }

Loop::Loop(SourceFile *src) : Stmt(ASTNodeType::LOOP, src) {}

void Loop::set_body(Stmt *body) { _body = body; }

void Loop::set_predicate(Expr *pred) { _predicate = pred; }

Expr *Loop::get_predicate() const { return _predicate; }

Stmt *Loop::get_body() const { return _body; }

vector<ASTBase *> Loop::get_children() const { return {(ASTBase *)_predicate, _body}; }

/// \section If-else

If::If(SourceFile *src) : Stmt(ASTNodeType::IF, src) {}

If *If::Create(SourceFile *src) { return new If(src); }

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

PackageStmt::PackageStmt(SourceFile *src) : Stmt(ASTNodeType::PACKAGE, src) {}

PackageStmt *PackageStmt::Create(SourceFile *src) { return new PackageStmt(src); }

str PackageStmt::get_name() const { return _name; }

void PackageStmt::set_name(const str &name) { _name = name; }
