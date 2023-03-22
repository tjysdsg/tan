#ifndef __TAN_SRC_ANALYSIS_NAMEDSYMBOLDEPENDENCYGRAPH_H__
#define __TAN_SRC_ANALYSIS_NAMEDSYMBOLDEPENDENCYGRAPH_H__

#include "base.h"

namespace tanlang {

class ASTBase;

class DependencyGraph {
public:
  /**
   * \brief \p dependent depends on \p depended
   */
  void add_dependency(ASTBase *depended, ASTBase *dependent);

  void clear();

private:
  umap<ASTBase *, vector<ASTBase *>> _dependencies{};
  umap<ASTBase *, vector<str>> _name_dependencies{};
};

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_NAMEDSYMBOLDEPENDENCYGRAPH_H__
