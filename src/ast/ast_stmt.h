#ifndef __TAN_SRC_AST_AST_STMT_H__
#define __TAN_SRC_AST_AST_STMT_H__
#include "base.h"
#include "src/base/container.h"
#include "src/ast/ast_base.h"

namespace tanlang {

AST_FWD_DECL(ASTStatement);

class ASTStatement : public ASTBase {
public:
  void set_child_at(size_t idx, ASTStatementPtr node);
  void append_child(ASTStatementPtr node);
  void clear_children();
  size_t get_children_size() const;
  vector<ASTStatementPtr> get_children() const;
  vector<ASTStatementPtr> &get_children();

  template<typename T> ptr<T> get_child_at(size_t idx) const {
    static_assert(std::is_base_of_v<ASTStatement, T>, "Return type can only be a subclass of ASTStatement");
    TAN_ASSERT(_children.size() > idx);
    return ast_must_cast<T>(_children[idx]);
  }

  template<> ptr<ASTStatement> get_child_at<ASTStatement>(size_t idx) const {
    TAN_ASSERT(_children.size() > idx);
    return _children[idx];
  }

protected:
  vector<ASTStatementPtr> _children{};
};

}

#endif //__TAN_SRC_AST_AST_STMT_H__
