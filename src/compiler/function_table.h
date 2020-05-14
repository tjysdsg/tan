#ifndef __TAN_SRC_COMPILER_FUNCTION_TABLE_H__
#define __TAN_SRC_COMPILER_FUNCTION_TABLE_H__
#include "base.h"
#include <memory>

namespace tanlang {

class ASTFunction;
using ASTFunctionPtr = std::shared_ptr<ASTFunction>;

class FunctionTable final {
public:
  FunctionTable() = default;
  void set(ASTFunctionPtr func);
  vector<ASTFunctionPtr> get(const str &name);
  vector<ASTFunctionPtr> get_all() const;

private:
  umap<str, vector<ASTFunctionPtr>> _table{};
};

using FunctionTablePtr = std::shared_ptr<FunctionTable>;

} // namespace tanlang

#endif /* __TAN_SRC_COMPILER_FUNCTION_TABLE_H__ */
