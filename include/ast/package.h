#ifndef __TAN_SRC_ANALYSIS_PACKAGE_H__
#define __TAN_SRC_ANALYSIS_PACKAGE_H__

#include "base/container.h"
#include "ast/ast_base.h"
#include "common/dependency_graph.h"

namespace tanlang {

class ASTBase;

class CompilationUnit;

class Package : public ASTBase {
public:
  Package() = delete;
  Package(const str &name, vector<ASTBase *> subtrees);
  [[nodiscard]] vector<ASTBase *> get_children() const override;
  str get_name() const { return _name; }

public:
  DependencyGraph<ASTBase *> top_level_symbol_dependency{};

protected:
  str to_string(SourceManager *) const override;

private:
  str _name;
  vector<ASTBase *> _subtrees{};
};

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_PACKAGE_H__
