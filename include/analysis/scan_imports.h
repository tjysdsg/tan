#ifndef __TAN_SRC_ANALYSIS_SCAN_IMPORTS_H__
#define __TAN_SRC_ANALYSIS_SCAN_IMPORTS_H__

#include "analysis/analysis_action.h"
#include "base/container.h"

namespace tanlang {

class Package;

/**
 * \brief Scans all dependencies in a package and return their names
 */
class ScanImports : public SemanticAnalysisAction<ScanImports, Package *, uset<str>> {
public:
  uset<str> run_impl(Package *package);
};

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_SCAN_IMPORTS_H__
