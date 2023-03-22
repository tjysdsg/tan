#include "analysis/dependency_graph.h"

using namespace tanlang;

void DependencyGraph::clear() { _dependencies.clear(); }

void DependencyGraph::add_dependency(ASTBase *depended, ASTBase *dependent) {
  _dependencies[dependent].push_back(depended);
}
