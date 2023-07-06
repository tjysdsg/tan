#ifndef __TAN_SRC_AST_STMT_H__
#define __TAN_SRC_AST_STMT_H__
#include "base.h"
#include "ast_base.h"
#include "fwd.h"

namespace llvm {
class BasicBlock;
}

namespace tanlang {

class Stmt : public ASTBase {
public:
  [[nodiscard]] vector<ASTBase *> get_children() const override;

  bool is_stmt() const override { return true; }

protected:
  Stmt(ASTNodeType type, SourceFile *src);
};

class CompoundStmt : public Stmt {
protected:
  explicit CompoundStmt(SourceFile *src);

public:
  static CompoundStmt *Create(SourceFile *src);

  void set_child_at(size_t idx, ASTBase *node);
  void append_child(ASTBase *node);
  void clear_children();
  [[nodiscard]] size_t get_children_size() const;
  [[nodiscard]] vector<ASTBase *> get_children() const override;
  vector<ASTBase *> &get_children();

  str terminal_token() const override { return "}"; }

protected:
  str to_string(SourceManager *) const override { return ASTBase::to_string(); }

protected:
  vector<ASTBase *> _children{};
};

class Program : public CompoundStmt {
protected:
  explicit Program(SourceFile *src);

public:
  static Program *Create(SourceFile *src);
};

class Return : public Stmt {
protected:
  explicit Return(SourceFile *src);

public:
  static Return *Create(SourceFile *src);

  void set_rhs(Expr *rhs);
  [[nodiscard]] Expr *get_rhs() const;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

private:
  Expr *_rhs = nullptr;
};

class Import : public Stmt {
protected:
  explicit Import(SourceFile *src);

public:
  static Import *Create(SourceFile *src);

  void set_filename(const str &s);
  [[nodiscard]] const str &get_filename() const;
  [[nodiscard]] const vector<FunctionDecl *> &get_imported_funcs() const;
  void set_imported_funcs(const vector<FunctionDecl *> &imported_funcs);

private:
  str _filename;
  vector<FunctionDecl *> _imported_funcs{};
};

class BreakContinue : public Stmt {
protected:
  BreakContinue(ASTNodeType type, SourceFile *src);

public:
  [[nodiscard]] Loop *get_parent_loop() const;
  void set_parent_loop(Loop *parent_loop);

private:
  Loop *_parent_loop = nullptr;
};

class Break : public BreakContinue {
protected:
  explicit Break(SourceFile *src);

public:
  static Break *Create(SourceFile *src);
};

class Continue : public BreakContinue {
protected:
  explicit Continue(SourceFile *src);

public:
  static Continue *Create(SourceFile *src);
};

enum ASTLoopType { FOR, WHILE };

class Loop final : public Stmt {
protected:
  explicit Loop(SourceFile *src);

public:
  static Loop *Create(SourceFile *src);

public:
  void set_predicate(Expr *pred);
  void set_body(Stmt *body);
  [[nodiscard]] Expr *get_predicate() const;
  [[nodiscard]] Stmt *get_body() const;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

public:
  str terminal_token() const override { return "}"; }

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
  explicit If(SourceFile *src);

public:
  static If *Create(SourceFile *src);

public:
  str terminal_token() const override { return "}"; }

public:
  void add_if_then_branch(Expr *pred, Stmt *branch);
  void add_else_branch(Stmt *branch);

  /**
   * \note Return value can be a nullptr if the branch is an "else"
   */
  [[nodiscard]] Expr *get_predicate(size_t i) const;

  void set_predicate(size_t i, Expr *expr);

  [[nodiscard]] Stmt *get_branch(size_t i) const;
  [[nodiscard]] size_t get_num_branches() const;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

private:
  /// \note The last element can be a nullptr if the last branch is an "else"
  vector<Expr *> _predicates{};
  vector<Stmt *> _branches{};
};

class PackageStmt : public Stmt {
protected:
  explicit PackageStmt(SourceFile *src);

public:
  static PackageStmt *Create(SourceFile *src);

public:
  str get_name() const;
  void set_name(const str &name);

private:
  str _name = "";
};

} // namespace tanlang

#endif //__TAN_SRC_AST_STMT_H__
