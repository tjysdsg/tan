#ifndef __TAN_SRC_COMPILER_FUNCTION_TABLE_H__
#define __TAN_SRC_COMPILER_FUNCTION_TABLE_H__
#include <memory>
#include <unordered_map>
#include "src/ast/ast_ty.h"
#include "src/ast/ast_func.h"

namespace tanlang {

class FunctionTable final {
public:
  FunctionTable() = default;
  void set(ASTFunctionPtr func);
  std::vector<ASTFunctionPtr> get(const std::string &name);
  std::vector<ASTFunctionPtr> get_all() const;

private:
  std::unordered_map<std::string, std::vector<ASTFunctionPtr>> _table{};
};

using FunctionTablePtr = std::shared_ptr<FunctionTable>;

} // namespace tanlang

#endif /* __TAN_SRC_COMPILER_FUNCTION_TABLE_H__ */
