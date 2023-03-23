#include "analysis/dependency_graph.h"
#include <queue>

using namespace tanlang;

void DependencyGraph::clear() {
  _forward.clear();
  _backward.clear();
  _all.clear();
}

void DependencyGraph::add_dependency(ASTBase *depended, ASTBase *dependent) {
  _forward[dependent].push_back(depended);
  _backward[depended].push_back(dependent);
  _all.insert(depended);
  _all.insert(dependent);
}

vector<ASTBase *> DependencyGraph::topological_sort() const {
  umap<ASTBase *, int> num_depend{};
  std::queue<ASTBase *> q{};

  for (ASTBase *node : _all) {
    int n = num_depended(node);
    if (n == 0) {
      q.push(node);
    }
    num_depend[node] = n;
  }

  vector<ASTBase *> ret{};
  while (!q.empty()) {
    ASTBase *curr = q.front();
    q.pop();
    ret.push_back(curr);

    auto f = _backward.find(curr);
    vector<ASTBase *> depended{};
    if (f != _backward.end()) {
      depended = f->second;
    }
    for (auto *d : depended) {
      --num_depend[d];
      if (num_depend[d] == 0) {
        q.push(d);
      }
    }
  }

  // for (auto [d1, d2] : _forward) {
  //   str d1name = ast_cast<Decl>(d1)->get_name();
  //   for (auto *d : d2) {
  //     str d2name = ast_cast<Decl>(d)->get_name();
  //     std::cout << fmt::format("{} depends on {}\n", d1name, d2name);
  //   }
  // }

  // check if cyclic
  for (auto [node, n_depend] : num_depend) {
    if (n_depend)
      return {};
  }

  return ret;
}

int DependencyGraph::num_dependent(ASTBase *depended) const {
  auto q = _backward.find(depended);
  if (q == _backward.end()) {
    return 0;
  } else {
    return (int)q->second.size();
  }
}

int DependencyGraph::num_depended(ASTBase *dependent) const {
  auto q = _forward.find(dependent);
  if (q == _forward.end()) {
    return 0;
  } else {
    return (int)q->second.size();
  }
}
