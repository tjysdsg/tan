#include "src/ast/stmt.h"

using namespace tanlang;

/// \section Stmt

Stmt::Stmt(ASTNodeType type) : ASTBase(type, PREC_LOWEST) {}

/// \section Compound statement

ptr<CompoundStmt> CompoundStmt::Create() { return make_ptr<CompoundStmt>(); }

CompoundStmt::CompoundStmt() : Stmt(ASTNodeType::STATEMENT) {}

void CompoundStmt::set_child_at(size_t idx, ASTBasePtr node) {
  TAN_ASSERT(_children.size() > idx);
  _children[idx] = node;
}

void CompoundStmt::append_child(ASTBasePtr node) { _children.push_back(node); }

void CompoundStmt::clear_children() { _children.clear(); }

size_t CompoundStmt::get_children_size() const { return _children.size(); }

vector<ASTBasePtr> &CompoundStmt::get_children() { return _children; }

template<typename T> ptr<T> CompoundStmt::get_child_at(size_t idx) const {
  static_assert(std::is_base_of_v<Stmt, T>, "Return type can only be a subclass of Stmt");
  TAN_ASSERT(_children.size() > idx);
  return ast_must_cast<T>(_children[idx]);
}

template<> ASTBasePtr CompoundStmt::get_child_at<ASTBase>(size_t idx) const {
  TAN_ASSERT(_children.size() > idx);
  return _children[idx];
}

vector<ASTBasePtr> CompoundStmt::get_children() const { return _children; }

/// \section Program

ptr<Program> Program::Create() { return make_ptr<Program>(); }

Program::Program() { set_node_type((ASTNodeType::PROGRAM)); }

ptr<Return> Return::Create() { return make_ptr<Return>(); }

/// \section Return

Return::Return() : Stmt(ASTNodeType::RET) {}

void Return::set_rhs(ExprPtr rhs) { _rhs = rhs; }

const ExprPtr &Return::get_rhs() const { return _rhs; }

/// \section Import

ptr<Import> Import::Create() { return make_ptr<Import>(); }

Import::Import() : Stmt(ASTNodeType::IMPORT) {}

void Import::set_filename(const str& s) { _filename = s; }

const str &Import::get_filename() const { return _filename; }

const vector<FunctionDeclPtr> &Import::get_imported_funcs() const { return _imported_funcs; }

void Import::set_imported_funcs(const vector<FunctionDeclPtr> &imported_funcs) { _imported_funcs = imported_funcs; }

/// \section Break or continue statement

BreakContinue::BreakContinue(ASTNodeType type) : Stmt(type) {
  TAN_ASSERT(type == ASTNodeType::BREAK || type == ASTNodeType::CONTINUE);
}

ptr<Break> Break::Create() { return make_ptr<Break>(); }

ptr<Continue> Continue::Create() { return make_ptr<Continue>(); }

/// \section Loop

ptr<Loop> Loop::Create() { return make_ptr<Loop>(); }

Loop::Loop() : Stmt(ASTNodeType::LOOP) {}

void Loop::set_body(StmtPtr body) { _body = body; }

void Loop::set_predicate(ExprPtr pred) { _predicate = pred; }

const ExprPtr &Loop::get_predicate() const { return _predicate; }

const StmtPtr &Loop::get_body() const { return _body; }

/// \section If-else

If::If() : Stmt(ASTNodeType::IF) {}

ptr<If> If::Create() { return make_ptr<If>(); }

void If::set_predicate(ExprPtr pred) { _predicate = pred; }

void If::set_then(StmtPtr body) { _then = body; }

void If::set_else(StmtPtr body) { _else = body; }
