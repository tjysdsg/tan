#include "src/ast/ast_stmt.h"

using namespace tanlang;

void ASTStatement::set_child_at(size_t idx, ptr<ASTStatement> node) {
  TAN_ASSERT(_children.size() > idx);
  _children[idx] = node;
}

void ASTStatement::append_child(ptr<ASTStatement> node) {
  _children.push_back(node);
}

void ASTStatement::clear_children() {
  _children.clear();
}

size_t ASTStatement::get_children_size() const {
  return _children.size();
}

vector<ASTStatementPtr> &ASTStatement::get_children() {
  return _children;
}

vector<ASTStatementPtr> ASTStatement::get_children() const {
  return _children;
}
