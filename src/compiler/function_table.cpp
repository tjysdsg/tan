#include "src/compiler/function_table.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_ty.h"

namespace tanlang {

void FunctionTable::set(ASTFunctionPtr func) {
  auto name = func->get_name();
  if (_table.find(name) == _table.end()) { _table[name] = {}; }
  _table[name].push_back(func);
}

vector<ASTFunctionPtr> FunctionTable::get(const str &name) {
  vector<ASTFunctionPtr> ret{};
  if (_table.find(name) != _table.end()) {
    ret.insert(ret.end(), _table[name].begin(), _table[name].end());
  }
  return ret;
}

vector<ASTFunctionPtr> FunctionTable::get_all() const {
  vector<ASTFunctionPtr> ret{};
  ret.reserve(_table.size());
  for (const auto &p: _table) {
    ret.insert(ret.end(), p.second.begin(), p.second.end());
  }
  return ret;
}

} // namespace tanlang
