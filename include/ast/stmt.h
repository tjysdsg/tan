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
  bool is_expr() const override { return false; }

protected:
  Stmt(ASTNodeType type, TokenizedSourceFile *src);
};

class CompoundStmt : public Stmt {
protected:
  explicit CompoundStmt(TokenizedSourceFile *src);

public:
  static CompoundStmt *Create(TokenizedSourceFile *src);

  void set_child_at(size_t idx, ASTBase *node);
  void append_child(ASTBase *node);
  void clear_children();
  [[nodiscard]] size_t get_children_size() const;
  [[nodiscard]] vector<ASTBase *> get_children() const override;
  vector<ASTBase *> &get_children();

  str terminal_token() const override { return "}"; }

protected:
  str to_string(bool = false) const override { return Stmt::to_string(false); }

protected:
  vector<ASTBase *> _children{};
};

class Program : public CompoundStmt {
protected:
  explicit Program(TokenizedSourceFile *src);

public:
  static Program *Create(TokenizedSourceFile *src);
};

class Return : public Stmt {
protected:
  explicit Return(TokenizedSourceFile *src);

public:
  static Return *Create(TokenizedSourceFile *src);

  void set_rhs(Expr *rhs);
  [[nodiscard]] Expr *get_rhs() const;

  [[nodiscard]] vector<ASTBase *> get_children() const override;

private:
  Expr *_rhs = nullptr;
};

class Import : public Stmt {
protected:
  explicit Import(TokenizedSourceFile *src);

public:
  static Import *Create(TokenizedSourceFile *src);

  void set_name(const str &s);
  [[nodiscard]] const str &get_name() const;

public:
  vector<FunctionDecl *> _imported_funcs{};
  vector<TypeDecl *> _imported_types{};

private:
  str _name;
};

class BreakContinue : public Stmt {
protected:
  BreakContinue(ASTNodeType type, TokenizedSourceFile *src);

public:
  [[nodiscard]] Loop *get_parent_loop() const;
  void set_parent_loop(Loop *parent_loop);

private:
  Loop *_parent_loop = nullptr;
};

class Break : public BreakContinue {
protected:
  explicit Break(TokenizedSourceFile *src);

public:
  static Break *Create(TokenizedSourceFile *src);
};

class Continue : public BreakContinue {
protected:
  explicit Continue(TokenizedSourceFile *src);

public:
  static Continue *Create(TokenizedSourceFile *src);
};

enum class ASTLoopType { FOR, WHILE };

class Loop final : public Stmt {
protected:
  explicit Loop(TokenizedSourceFile *src);

public:
  static Loop *Create(TokenizedSourceFile *src);

  [[nodiscard]] vector<ASTBase *> get_children() const override;

  str terminal_token() const override { return "}"; }

public:
  ASTLoopType _loop_type = ASTLoopType::WHILE;
  llvm::BasicBlock *_loop_start = nullptr;
  llvm::BasicBlock *_loop_end = nullptr;

  Expr *_initialization = nullptr;
  Expr *_predicate = nullptr;
  Stmt *_body = nullptr;
  Expr *_iteration = nullptr;
};

/**
 * \brief Represent if-[else] or if-elif-[else] statements
 */
class If : public Stmt {
protected:
  explicit If(TokenizedSourceFile *src);

public:
  static If *Create(TokenizedSourceFile *src);

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

class PackageDecl : public Stmt {
protected:
  explicit PackageDecl(TokenizedSourceFile *src);

public:
  static PackageDecl *Create(TokenizedSourceFile *src);

public:
  str get_name() const;
  void set_name(const str &name);

private:
  str _name = "";
};

} // namespace tanlang

#endif //__TAN_SRC_AST_STMT_H__
