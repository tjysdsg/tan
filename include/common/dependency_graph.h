#ifndef __TAN_SRC_ANALYSIS_DEPENDENCY_GRAPH_H__
#define __TAN_SRC_ANALYSIS_DEPENDENCY_GRAPH_H__

#include "base.h"
#include <unordered_set>
#include <queue>
#include <optional>

namespace tanlang {

template <typename T> class DependencyGraph {
public:
  /**
   * \brief \p dependent depends on \p depended
   */
  void add_dependency(T depended, T dependent) {
    _forward[dependent].push_back(depended);
    _backward[depended].push_back(dependent);
    _all.insert(depended);
    _all.insert(dependent);
  }

  /**
   * \brief Sort topologically so for no element is dependent on its succeeding element(s).
   * \return (res, node)
   *         - If successful, res is a sorted vector of T and node is nullopt
   *         - Otherwise, res is nullopt, and node is the node that has cyclic dependency
   */
  std::pair<std::optional<vector<T>>, std::optional<T>> topological_sort() const {
    umap<T, int> num_depend{};
    std::queue<T> q{};

    for (T node : _all) {
      int n = num_depended(node);
      if (n == 0) {
        q.push(node);
      }
      num_depend[node] = n;
    }

    vector<T> ret{};
    while (!q.empty()) {
      T curr = q.front();
      q.pop();
      ret.push_back(curr);

      auto f = _backward.find(curr);
      vector<T> depended{};
      if (f != _backward.end()) {
        depended = f->second;
      }
      for (auto d : depended) {
        --num_depend[d];
        if (num_depend[d] == 0) {
          q.push(d);
        }
      }
    }

    // for (auto [d1, d2] : _forward) {
    //   str d1name = pcast<Decl>(d1)->get_name();
    //   for (auto *d : d2) {
    //     str d2name = pcast<Decl>(d)->get_name();
    //     std::cout << fmt::format("{} depends on {}\n", d1name, d2name);
    //   }
    // }

    // check if cyclic
    for (auto [node, n_depend] : num_depend) {
      if (n_depend)
        return std::make_pair(std::nullopt, node);
    }

    return std::make_pair(ret, std::nullopt);
  }

  /**
   * \brief Number of nodes that depends on \p depended.
   */
  int num_dependent(T depended) const {
    auto q = _backward.find(depended);
    if (q == _backward.end()) {
      return 0;
    } else {
      return (int)q->second.size();
    }
  }

  /**
   * \brief Number of nodes that \p dependent depends on.
   */
  int num_depended(T dependent) const {
    auto q = _forward.find(dependent);
    if (q == _forward.end()) {
      return 0;
    } else {
      return (int)q->second.size();
    }
  }

  void clear() {
    _forward.clear();
    _backward.clear();
    _all.clear();
  }

private:
  umap<T, vector<T>> _forward{};  // dependent -> depended
  umap<T, vector<T>> _backward{}; // depended -> dependent
  std::unordered_set<T> _all{};
};

} // namespace tanlang

#endif // __TAN_SRC_ANALYSIS_DEPENDENCY_GRAPH_H__