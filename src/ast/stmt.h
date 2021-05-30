#ifndef __TAN_SRC_AST_STMT_H__
#define __TAN_SRC_AST_STMT_H__
#include "base.h"
#include "src/ast/ast_base.h"

namespace tanlang {

AST_FWD_DECL(Stmt);

class Stmt : public ASTBase {
public:
  static ptr<Stmt> Create();
  Stmt();

public:
  void set_child_at(size_t idx, ASTBasePtr node);
  void append_child(ASTBasePtr node);
  void clear_children();
  size_t get_children_size() const;
  vector<ASTBasePtr> get_children() const;
  vector<ASTBasePtr> &get_children();
  template<typename T = ASTBase> ptr<T> get_child_at(size_t idx) const;

protected:
  vector<ASTBasePtr> _children{};
};

class Program : public Stmt {
public:
  static ptr<Program> Create();
  Program();
};

AST_FWD_DECL(Expr);

class Return : public Stmt {
public:
  static ptr<Return> Create();
  Return();
  void set_rhs(ExprPtr rhs);

private:
  ExprPtr _rhs = nullptr;
};

class Import : public Stmt {
public:
  static ptr<Import> Create();
  Import();
  void set_filename(str_view s);

private:
  str _filename;
};

class BreakContinue : public Stmt {
public:
  BreakContinue(ASTNodeType type);
};

class Break : public BreakContinue {
public:
  static ptr<Break> Create();
  Break() : BreakContinue(ASTNodeType::BREAK) {}
};

class Continue : public BreakContinue {
public:
  static ptr<Continue> Create();
  Continue() : BreakContinue(ASTNodeType::CONTINUE) {}
};

enum ASTLoopType { FOR, WHILE };

class Loop final : public Stmt {
public:
  static ptr<Loop> Create();
  Loop();

  void set_predicate(ExprPtr pred);
  void set_body(StmtPtr body);

public:
  ASTLoopType _loop_type = ASTLoopType::WHILE;
  ExprPtr _predicate = nullptr;
  StmtPtr _body = nullptr;
  // llvm::BasicBlock *_loop_start = nullptr;
  // llvm::BasicBlock *_loop_end = nullptr;
};

class If : public Stmt {
public:
  static ptr<If> Create();
  If();
  void set_predicate(ExprPtr pred);
  void set_then(StmtPtr body);
  void set_else(StmtPtr body);

private:
  ExprPtr _predicate = nullptr;
  StmtPtr _then = nullptr;
  StmtPtr _else = nullptr;
};

}

#endif //__TAN_SRC_AST_STMT_H__
