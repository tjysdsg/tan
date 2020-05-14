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
  std::vector<ASTFunctionPtr> get(const str &name);
  std::vector<ASTFunctionPtr> get_all() const;

private:
  std::unordered_map<str, std::vector<ASTFunctionPtr>> _table{};
};

using FunctionTablePtr = std::shared_ptr<FunctionTable>;

} // namespace tanlang

#endif /* __TAN_SRC_COMPILER_FUNCTION_TABLE_H__ */
