#ifndef __TAN_SRC_AST_STMT_H__
#define __TAN_SRC_AST_STMT_H__
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/fwd.h"

namespace llvm {
class BasicBlock;
}

namespace tanlang {

class Stmt : public ASTBase {
protected:
  Stmt(ASTNodeType type, SourceIndex loc);
};

class CompoundStmt : public Stmt {
protected:
  CompoundStmt(SourceIndex loc);

public:
  static CompoundStmt *Create(SourceIndex loc);

  void set_child_at(size_t idx, ASTBase *node);
  void append_child(ASTBase *node);
  void clear_children();
  size_t get_children_size() const;
  vector<ASTBase *> get_children() const override;
  vector<ASTBase *> &get_children();
  template<typename T = ASTBase> T *get_child_at(size_t idx) const;

protected:
  vector<ASTBase *> _children{};
};

class Program : public CompoundStmt {
protected:
  Program(SourceIndex loc);

public:
  static Program *Create(SourceIndex loc);
};

class Return : public Stmt {
protected:
  Return(SourceIndex loc);

public:
  static Return *Create(SourceIndex loc);

  void set_rhs(Expr *rhs);
  Expr *get_rhs() const;

  vector<ASTBase *> get_children() const override;

private:
  Expr *_rhs = nullptr;
};

class Import : public Stmt {
protected:
  Import(SourceIndex loc);

public:
  static Import *Create(SourceIndex loc);

  void set_filename(const str &s);
  const str &get_filename() const;
  const vector<FunctionDecl *> &get_imported_funcs() const;
  void set_imported_funcs(const vector<FunctionDecl *> &imported_funcs);

private:
  str _filename;
  vector<FunctionDecl *> _imported_funcs{};
};

class BreakContinue : public Stmt {
protected:
  BreakContinue(ASTNodeType type, SourceIndex loc);

public:
  Loop *get_parent_loop() const;
  void set_parent_loop(Loop *parent_loop);

private:
  Loop *_parent_loop = nullptr;
};

class Break : public BreakContinue {
protected:
  Break(SourceIndex loc);

public:
  static Break *Create(SourceIndex loc);
};

class Continue : public BreakContinue {
protected:
  Continue(SourceIndex loc);

public:
  static Continue *Create(SourceIndex loc);
};

enum ASTLoopType { FOR, WHILE };

class Loop final : public Stmt {
protected:
  Loop(SourceIndex loc);

public:
  static Loop *Create(SourceIndex loc);

  void set_predicate(Expr *pred);
  void set_body(Stmt *body);
  Expr *get_predicate() const;
  Stmt *get_body() const;

  vector<ASTBase *> get_children() const override;

public:
  ASTLoopType _loop_type = ASTLoopType::WHILE;
  llvm::BasicBlock *_loop_start = nullptr;
  llvm::BasicBlock *_loop_end = nullptr;

private:
  Expr *_predicate = nullptr;
  Stmt *_body = nullptr;
};

/**
 * \brief Represent if-[else] or if-elif-[else] statements
 */
class If : public Stmt {
protected:
  If(SourceIndex loc);

public:
  static If *Create(SourceIndex loc);

public:
  void add_if_then_branch(Expr *pred, Stmt *branch);
  void add_else_branch(Stmt *branch);

  /**
   * \note Return value can be a nullptr if the branch is an "else"
   */
  Expr *get_predicate(size_t i) const;

  Stmt *get_branch(size_t i) const;
  size_t get_num_branches() const;
  bool is_last_branch_else() const;

  vector<ASTBase *> get_children() const override;

private:
  /// \note The last element can be a nullptr if the last branch is an "else"
  vector<Expr *> _predicates{};
  vector<Stmt *> _branches{};

  /// \brief When true, the last branch is an "else"
  bool _last_branch_else = false;
};

}

#endif //__TAN_SRC_AST_STMT_H__
