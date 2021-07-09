#include "src/ast/stmt.h"

using namespace tanlang;

/// \section Stmt

Stmt::Stmt(ASTNodeType type, SourceIndex loc) : ASTBase(type, loc, PREC_LOWEST) {}

/// \section Compound statement

CompoundStmt *CompoundStmt::Create(SourceIndex loc) { return new CompoundStmt(loc); }

CompoundStmt::CompoundStmt(SourceIndex loc) : Stmt(ASTNodeType::STATEMENT, loc) {}

void CompoundStmt::set_child_at(size_t idx, ASTBase *node) {
  TAN_ASSERT(_children.size() > idx);
  _children[idx] = node;
}

void CompoundStmt::append_child(ASTBase *node) { _children.push_back(node); }

void CompoundStmt::clear_children() { _children.clear(); }

size_t CompoundStmt::get_children_size() const { return _children.size(); }

vector<ASTBase *> &CompoundStmt::get_children() { return _children; }

template<typename T> T *CompoundStmt::get_child_at(size_t idx) const {
  static_assert(std::is_base_of_v<Stmt, T>, "Return type can only be a subclass of Stmt");
  TAN_ASSERT(_children.size() > idx);
  return ast_must_cast<T>(_children[idx]);
}

template<> ASTBase *CompoundStmt::get_child_at<ASTBase>(size_t idx) const {
  TAN_ASSERT(_children.size() > idx);
  return _children[idx];
}

vector<ASTBase *> CompoundStmt::get_children() const { return _children; }

/// \section Program

Program *Program::Create(SourceIndex loc) { return new Program(loc); }

Program::Program(SourceIndex loc) : CompoundStmt(loc) { set_node_type((ASTNodeType::PROGRAM)); }

Return *Return::Create(SourceIndex loc) { return new Return(loc); }

/// \section Return

Return::Return(SourceIndex loc) : Stmt(ASTNodeType::RET, loc) {}

void Return::set_rhs(Expr *rhs) { _rhs = rhs; }

Expr *Return::get_rhs() const { return _rhs; }

/// \section Import

Import *Import::Create(SourceIndex loc) { return new Import(loc); }

Import::Import(SourceIndex loc) : Stmt(ASTNodeType::IMPORT, loc) {}

void Import::set_filename(const str &s) { _filename = s; }

const str &Import::get_filename() const { return _filename; }

const vector<FunctionDecl *> &Import::get_imported_funcs() const { return _imported_funcs; }

void Import::set_imported_funcs(const vector<FunctionDecl *> &imported_funcs) { _imported_funcs = imported_funcs; }

/// \section Break or continue statement

BreakContinue::BreakContinue(ASTNodeType type, SourceIndex loc) : Stmt(type, loc) {
  TAN_ASSERT(type == ASTNodeType::BREAK || type == ASTNodeType::CONTINUE);
}

Loop *BreakContinue::get_parent_loop() const { return _parent_loop; }

void BreakContinue::set_parent_loop(Loop *parent_loop) { _parent_loop = parent_loop; }

Break *Break::Create(SourceIndex loc) { return new Break(loc); }

Break::Break(SourceIndex loc) : BreakContinue(ASTNodeType::BREAK, loc) {}

Continue *Continue::Create(SourceIndex loc) { return new Continue(loc); }

Continue::Continue(SourceIndex loc) : BreakContinue(ASTNodeType::CONTINUE, loc) {}

/// \section Loop

Loop *Loop::Create(SourceIndex loc) { return new Loop(loc); }

Loop::Loop(SourceIndex loc) : Stmt(ASTNodeType::LOOP, loc) {}

void Loop::set_body(Stmt *body) { _body = body; }

void Loop::set_predicate(Expr *pred) { _predicate = pred; }

Expr *Loop::get_predicate() const { return _predicate; }

Stmt *Loop::get_body() const { return _body; }

/// \section If-else

If::If(SourceIndex loc) : Stmt(ASTNodeType::IF, loc) {}

If *If::Create(SourceIndex loc) { return new If(loc); }

void If::add_if_then_branch(Expr *pred, Stmt *branch) {
  TAN_ASSERT(pred);
  _predicates.push_back(pred);
  _branches.push_back(branch);
}

void If::add_else_branch(Stmt *branch) {
  _predicates.push_back(nullptr);
  _branches.push_back(branch);
  _last_branch_else = true;
}

Expr *If::get_predicate(size_t i) const {
  TAN_ASSERT(i < _predicates.size());
  return _predicates[i];
}

Stmt *If::get_branch(size_t i) const {
  TAN_ASSERT(i < _branches.size());
  return _branches[i];
}

bool If::is_last_branch_else() const { return _last_branch_else; }

size_t If::get_num_branches() const { return _branches.size(); }

