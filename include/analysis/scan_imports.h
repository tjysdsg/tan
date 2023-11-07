#ifndef __TAN_SRC_ANALYSIS_SCAN_IMPORTS_H__
#define __TAN_SRC_ANALYSIS_SCAN_IMPORTS_H__

#include "analysis/analysis_action.h"
#include "base/container.h"

namespace tanlang {

class Package;

// package name => [file paths]
using ScanImportsOutputType = umap<str, uset<str>>;

/**
 * \brief Scans all dependencies in a package, and return their names and paths to relevant source files
 */
class ScanImports : public SemanticAnalysisAction<ScanImports, Package *, ScanImportsOutputType> {
public:
  ScanImportsOutputType run_impl(Package *package);
};

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_SCAN_IMPORTS_H__
