#ifndef __TAN_SRC_ANALYSIS_ORGANIZE_PACKAGES_H__
#define __TAN_SRC_ANALYSIS_ORGANIZE_PACKAGES_H__

#include "common/compiler_action.h"
#include "base/container.h"

namespace tanlang {

class Package;

/**
 * \brief Organize a list of source files into their corresponding packages according to the code.
 *        Run this stage early since semantic analysis is performed on the package level.
 * \note The input will likely contain outdated information so any CompilerAction pass this should only operates
 * on the package-level.
 */
class OrganizePackages : public CompilerAction<OrganizePackages, vector<Program *>, vector<Package *>> {
public:
  vector<Package *> run_impl(vector<Program *> ps);

  DECLARE_AST_VISITOR_IMPL(Program);
  DECLARE_AST_VISITOR_IMPL(Intrinsic);

private:
  // package name => top level context
  umap<str, vector<Context *>> _package_top_level_ctx{};
  umap<str, vector<ASTBase *>> _package_top_level_asts{};
};

} // namespace tanlang

#endif // __TAN_SRC_ANALYSIS_ORGANIZE_PACKAGES_H__
