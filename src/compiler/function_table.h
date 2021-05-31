#ifndef __TAN_SRC_COMPILER_FUNCTION_TABLE_H__
#define __TAN_SRC_COMPILER_FUNCTION_TABLE_H__
#include "base.h"

namespace tanlang {

AST_FWD_DECL(FunctionDecl);

class FunctionTable final {
public:
  FunctionTable() = default;
  void set(FunctionDeclPtr func);
  vector<FunctionDeclPtr> get(const str &name);
  [[nodiscard]] vector<FunctionDeclPtr> get_all() const;

private:
  umap<str, vector<FunctionDeclPtr>> _table{};
};

using FunctionTablePtr = std::shared_ptr<FunctionTable>;

} // namespace tanlang

#endif /* __TAN_SRC_COMPILER_FUNCTION_TABLE_H__ */
