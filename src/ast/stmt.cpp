#include "src/ast/stmt.h"

using namespace tanlang;

/// \section Stmt

void Stmt::set_child_at(size_t idx, ptr<Stmt> node) {
  TAN_ASSERT(_children.size() > idx);
  _children[idx] = node;
}

void Stmt::append_child(ptr<Stmt> node) {
  _children.push_back(node);
}

void Stmt::clear_children() {
  _children.clear();
}

size_t Stmt::get_children_size() const {
  return _children.size();
}

vector<StmtPtr> &Stmt::get_children() {
  return _children;
}

vector<StmtPtr> Stmt::get_children() const {
  return _children;
}

StmtPtr Stmt::Create() { return make_ptr<Stmt>(); }

Stmt::Stmt() : ASTBase(ASTNodeType::STATEMENT, 0) {}

/// \section Program

ptr<Program> Program::Create() { return make_ptr<Program>(); }

Program::Program() { set_node_type(ASTNodeType::PROGRAM); }

ptr<Return> Return::Create() {
  return make_ptr<Return>();
}

Return::Return() {
  set_node_type(ASTNodeType::RET);
}

void Return::set_rhs(ExprPtr rhs) { _rhs = rhs; }

/// \section Import

ptr<Import> Import::Create() { return make_ptr<Import>(); }

Import::Import() { set_node_type(ASTNodeType::IMPORT); }

void Import::set_filename(str_view s) { _filename = s; }

/// \section Break or continue statement

BreakContinue::BreakContinue(ASTNodeType type) {
  TAN_ASSERT(type == ASTNodeType::BREAK || type == ASTNodeType::CONTINUE);
  set_node_type(type);
}

ptr<Break> Break::Create() { return make_ptr<Break>(); }

ptr<Continue> Continue::Create() { return make_ptr<Continue>(); }
