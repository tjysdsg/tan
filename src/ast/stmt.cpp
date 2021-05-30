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

Program::Program() : ASTBase(ASTNodeType::PROGRAM, 0) {}
