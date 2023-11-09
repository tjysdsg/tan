#ifndef __TAN_SRC_ANALYSIS_PACKAGE_H__
#define __TAN_SRC_ANALYSIS_PACKAGE_H__

#include "base/container.h"
#include "ast/ast_base.h"
#include "common/dependency_graph.h"

namespace tanlang {

class ASTBase;

class Package : public ASTBase {
public:
  Package() = delete;
  Package(const str &name, vector<ASTBase *> subtrees);
  [[nodiscard]] vector<ASTBase *> get_children() const override;
  str get_name() const { return _name; }

  bool is_expr() const override { return false; }
  bool is_stmt() const override { return false; }

public:
  DependencyGraph<ASTBase *> top_level_symbol_dependency{};

private:
  str _name;
  vector<ASTBase *> _subtrees{};
};

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_PACKAGE_H__
