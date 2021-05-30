#ifndef __TAN_SRC_AST_STMT_H__
#define __TAN_SRC_AST_STMT_H__
#include "base.h"
#include "src/ast/ast_base.h"

namespace tanlang {

AST_FWD_DECL(Stmt);

class Stmt : public ASTBase {
public:
  static StmtPtr Create();
  Stmt();

public:
  void set_child_at(size_t idx, StmtPtr node);
  void append_child(StmtPtr node);
  void clear_children();
  size_t get_children_size() const;
  vector<StmtPtr> get_children() const;
  vector<StmtPtr> &get_children();

  template<typename T> ptr<T> get_child_at(size_t idx) const {
    static_assert(std::is_base_of_v<Stmt, T>, "Return type can only be a subclass of Stmt");
    TAN_ASSERT(_children.size() > idx);
    return ast_must_cast<T>(_children[idx]);
  }

  template<> ptr<Stmt> get_child_at<Stmt>(size_t idx) const {
    TAN_ASSERT(_children.size() > idx);
    return _children[idx];
  }

protected:
  vector<StmtPtr> _children{};
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

class Loop final : public ASTBase {
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

}

#endif //__TAN_SRC_AST_STMT_H__
