#include "src/ast/stmt.h"

using namespace tanlang;

/// \section Stmt

void Stmt::set_child_at(size_t idx, ASTBasePtr node) {
  TAN_ASSERT(_children.size() > idx);
  _children[idx] = node;
}

void Stmt::append_child(ASTBasePtr node) { _children.push_back(node); }

void Stmt::clear_children() { _children.clear(); }

size_t Stmt::get_children_size() const { return _children.size(); }

vector<ASTBasePtr> &Stmt::get_children() { return _children; }

template<typename T> ptr<T> Stmt::get_child_at(size_t idx) const {
  static_assert(std::is_base_of_v<Stmt, T>, "Return type can only be a subclass of Stmt");
  TAN_ASSERT(_children.size() > idx);
  return ast_must_cast<T>(_children[idx]);
}

template<> ptr<Stmt> Stmt::get_child_at<Stmt>(size_t idx) const {
  TAN_ASSERT(_children.size() > idx);
  return _children[idx];
}

vector<ASTBasePtr> Stmt::get_children() const { return _children; }

ptr<Stmt> Stmt::Create() { return make_ptr<Stmt>(); }

Stmt::Stmt() : ASTBase(ASTNodeType::STATEMENT, 0) {}

/// \section Program

ptr<Program> Program::Create() { return make_ptr<Program>(); }

Program::Program() : ASTBase(ASTNodeType::PROGRAM, PREC_LOWEST) {}

ptr<Return> Return::Create() { return make_ptr<Return>(); }

/// \section Return

Return::Return() { set_node_type(ASTNodeType::RET); }

void Return::set_rhs(ExprPtr rhs) { _rhs = rhs; }

/// \section Import

ptr<Import> Import::Create() { return make_ptr<Import>(); }

Import::Import() { set_node_type(ASTNodeType::IMPORT); }

void Import::set_filename(str_view s) { _filename = s; }

const str &Import::get_filename() const { return _filename; }

/// \section Break or continue statement

BreakContinue::BreakContinue(ASTNodeType type) {
  TAN_ASSERT(type == ASTNodeType::BREAK || type == ASTNodeType::CONTINUE);
  set_node_type(type);
}

ptr<Break> Break::Create() { return make_ptr<Break>(); }

ptr<Continue> Continue::Create() { return make_ptr<Continue>(); }

/// \section Loop

ptr<Loop> Loop::Create() { return make_ptr<Loop>(); }

Loop::Loop() { set_node_type(ASTNodeType::LOOP); }

void Loop::set_body(StmtPtr body) { _body = body; }

void Loop::set_predicate(ExprPtr pred) { _predicate = pred; }

/// \section If-else

If::If() { set_node_type(ASTNodeType::IF); }

ptr<If> If::Create() { return make_ptr<If>(); }

void If::set_predicate(ExprPtr pred) { _predicate = pred; }

void If::set_then(StmtPtr body) { _then = body; }

void If::set_else(StmtPtr body) { _else = body; }

