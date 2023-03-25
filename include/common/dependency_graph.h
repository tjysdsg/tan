#ifndef __TAN_SRC_ANALYSIS_DEPENDENCY_GRAPH_H__
#define __TAN_SRC_ANALYSIS_DEPENDENCY_GRAPH_H__

#include "base.h"
#include <unordered_set>

namespace tanlang {

class ASTBase;

class DependencyGraph {
public:
  /**
   * \brief \p dependent depends on \p depended
   */
  void add_dependency(ASTBase *depended, ASTBase *dependent);

  /**
   * \brief Sort topologically so for no element is dependent on its succeeding element(s).
   * \return Empty vector if there is a dependency cycle.
   */
  vector<ASTBase *> topological_sort() const;

  /**
   * \brief Number of nodes that depends on \p depended.
   */
  int num_dependent(ASTBase *depended) const;

  /**
   * \brief Number of nodes that \p dependent depends on.
   */
  int num_depended(ASTBase *dependent) const;

  void clear();

private:
  umap<ASTBase *, vector<ASTBase *>> _forward{};  // dependent -> depended
  umap<ASTBase *, vector<ASTBase *>> _backward{}; // depended -> dependent
  std::unordered_set<ASTBase *> _all{};
};

} // namespace tanlang

#endif // __TAN_SRC_ANALYSIS_DEPENDENCY_GRAPH_H__