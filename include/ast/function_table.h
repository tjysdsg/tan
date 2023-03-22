#ifndef __TAN_SRC_COMPILER_FUNCTION_TABLE_H__
#define __TAN_SRC_COMPILER_FUNCTION_TABLE_H__
#include "base.h"

namespace tanlang {

class FunctionDecl;

class FunctionTable final {
public:
  FunctionTable() = default;
  void set(FunctionDecl *func);
  vector<FunctionDecl *> get(const str &name);
  [[nodiscard]] vector<FunctionDecl *> get_all() const;
  bool merge(const FunctionTable &other);

private:
  umap<str, vector<FunctionDecl *>> _table{};
};

} // namespace tanlang

#endif /* __TAN_SRC_COMPILER_FUNCTION_TABLE_H__ */
