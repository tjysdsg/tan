#ifndef __TAN_ANALYSIS_TYPE_CHECKER_H__
#define __TAN_ANALYSIS_TYPE_CHECKER_H__
#include "base.h"

namespace tanlang {

class Package;

class TypeChecker {
public:
  TypeChecker();
  ~TypeChecker();
  void type_check(Package *p, bool strict);

private:
  void *_impl = nullptr;
};

} // namespace tanlang

#endif // __TAN_ANALYSIS_TYPE_CHECKER_H__
