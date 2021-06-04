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
public:
  Stmt(ASTNodeType type);
};

class CompoundStmt : public Stmt {
public:
  static CompoundStmt *Create();
  CompoundStmt();

  void set_child_at(size_t idx, ASTBase *node);
  void append_child(ASTBase *node);
  void clear_children();
  size_t get_children_size() const;
  vector<ASTBase *> get_children() const;
  vector<ASTBase *> &get_children();
  template<typename T = ASTBase> T *get_child_at(size_t idx) const;

protected:
  vector<ASTBase *> _children{};
};

class Program : public CompoundStmt {
public:
  static Program *Create();
  Program();
};

class Return : public Stmt {
public:
  static Return *Create();
  Return();
  void set_rhs(Expr *rhs);
  Expr *get_rhs() const;

private:
  Expr *_rhs = nullptr;
};

class Import : public Stmt {
public:
  static Import *Create();
  Import();
  void set_filename(const str &s);
  const str &get_filename() const;
  const vector<FunctionDecl *> &get_imported_funcs() const;
  void set_imported_funcs(const vector<FunctionDecl *> &imported_funcs);

private:
  str _filename;
  vector<FunctionDecl *> _imported_funcs{};
};

class BreakContinue : public Stmt {
public:
  BreakContinue(ASTNodeType type);
};

class Break : public BreakContinue {
public:
  static Break *Create();
  Break() : BreakContinue(ASTNodeType::BREAK) {}
};

class Continue : public BreakContinue {
public:
  static Continue *Create();
  Continue() : BreakContinue(ASTNodeType::CONTINUE) {}
};

enum ASTLoopType { FOR, WHILE };

class Loop final : public Stmt {
public:
  static Loop *Create();
  Loop();

  void set_predicate(Expr *pred);
  void set_body(Stmt *body);
  Expr *get_predicate() const;
  Stmt *get_body() const;

public:
  ASTLoopType _loop_type = ASTLoopType::WHILE;
  llvm::BasicBlock *_loop_start = nullptr;
  llvm::BasicBlock *_loop_end = nullptr;

private:
  Expr *_predicate = nullptr;
  Stmt *_body = nullptr;
};

class If : public Stmt {
public:
  static If *Create();
  If();
  void set_predicate(Expr *pred);
  void set_then(Stmt *body);
  void set_else(Stmt *body);
  Expr *get_predicate() const { return _predicate; }
  Stmt *get_then() { return _then; }
  Stmt *get_else() { return _else; }

private:
  Expr *_predicate = nullptr;
  Stmt *_then = nullptr;
  Stmt *_else = nullptr;
};

}

#endif //__TAN_SRC_AST_STMT_H__
