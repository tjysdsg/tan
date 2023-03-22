#ifndef __TAN_ANALYSIS_TYPE_CHECKER_H__
#define __TAN_ANALYSIS_TYPE_CHECKER_H__
#include "base.h"

namespace tanlang {

class Package;
class Context;
class DependencyGraph;

class TypeChecker {
public:
  TypeChecker();
  ~TypeChecker();
  void type_check(Package *p, bool strict, const umap<str, Context *> &external_package_ctx);
  DependencyGraph get_unresolved_symbol_dependency() const;

private:
  void *_impl = nullptr;
};

} // namespace tanlang

#endif // __TAN_ANALYSIS_TYPE_CHECKER_H__
