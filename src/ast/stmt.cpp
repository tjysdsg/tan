#include "src/ast/stmt.h"

using namespace tanlang;

/// \section Stmt

Stmt::Stmt(ASTNodeType type) : ASTBase(type, PREC_LOWEST) {}

/// \section Compound statement

CompoundStmt *CompoundStmt::Create() { return new CompoundStmt; }

CompoundStmt::CompoundStmt() : Stmt(ASTNodeType::STATEMENT) {}

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

Program *Program::Create() { return new Program; }

Program::Program() { set_node_type((ASTNodeType::PROGRAM)); }

Return *Return::Create() { return new Return; }

/// \section Return

Return::Return() : Stmt(ASTNodeType::RET) {}

void Return::set_rhs(Expr *rhs) { _rhs = rhs; }

Expr *Return::get_rhs() const { return _rhs; }

/// \section Import

Import *Import::Create() { return new Import; }

Import::Import() : Stmt(ASTNodeType::IMPORT) {}

void Import::set_filename(const str &s) { _filename = s; }

const str &Import::get_filename() const { return _filename; }

const vector<FunctionDecl *> &Import::get_imported_funcs() const { return _imported_funcs; }

void Import::set_imported_funcs(const vector<FunctionDecl *> &imported_funcs) { _imported_funcs = imported_funcs; }

/// \section Break or continue statement

BreakContinue::BreakContinue(ASTNodeType type) : Stmt(type) {
  TAN_ASSERT(type == ASTNodeType::BREAK || type == ASTNodeType::CONTINUE);
}

Break *Break::Create() { return new Break; }

Continue *Continue::Create() { return new Continue; }

/// \section Loop

Loop *Loop::Create() { return new Loop; }

Loop::Loop() : Stmt(ASTNodeType::LOOP) {}

void Loop::set_body(Stmt *body) { _body = body; }

void Loop::set_predicate(Expr *pred) { _predicate = pred; }

Expr *Loop::get_predicate() const { return _predicate; }

Stmt *Loop::get_body() const { return _body; }

/// \section If-else

If::If() : Stmt(ASTNodeType::IF) {}

If *If::Create() { return new If; }

void If::set_predicate(Expr *pred) { _predicate = pred; }

void If::set_then(Stmt *body) { _then = body; }

void If::set_else(Stmt *body) { _else = body; }
