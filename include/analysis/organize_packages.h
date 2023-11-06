#ifndef __TAN_SRC_ANALYSIS_ORGANIZE_PACKAGES_H__
#define __TAN_SRC_ANALYSIS_ORGANIZE_PACKAGES_H__

#include "common/compiler_action.h"
#include "base/container.h"

namespace tanlang {

class CompilationUnit;
class Package;

class OrganizePackages : public CompilerAction<OrganizePackages, vector<CompilationUnit *>, vector<Package *>> {
public:
  vector<Package *> run_impl(vector<CompilationUnit *> cu);

  DECLARE_AST_VISITOR_IMPL(Program);
  DECLARE_AST_VISITOR_IMPL(Intrinsic);

private:
  // package name => top level context
  umap<str, vector<Context *>> _package_top_level_ctx{};
  umap<str, vector<ASTBase *>> _package_top_level_asts{};
};

} // namespace tanlang

#endif // __TAN_SRC_ANALYSIS_ORGANIZE_PACKAGES_H__
