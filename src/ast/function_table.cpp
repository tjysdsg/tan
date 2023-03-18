#include "ast/function_table.h"
#include "ast/type.h"
#include "ast/decl.h"

namespace tanlang {

void FunctionTable::set(FunctionDecl *func) {
  auto name = func->get_name();
  TAN_ASSERT(!name.empty());
  _table[name].push_back(func);
}

vector<FunctionDecl *> FunctionTable::get(const str &name) {
  vector<FunctionDecl *> ret{};
  if (_table.find(name) != _table.end()) {
    ret.insert(ret.end(), _table[name].begin(), _table[name].end());
  }
  return ret;
}

vector<FunctionDecl *> FunctionTable::get_all() const {
  vector<FunctionDecl *> ret{};
  ret.reserve(_table.size());
  for (const auto &p : _table) {
    ret.insert(ret.end(), p.second.begin(), p.second.end());
  }
  return ret;
}

} // namespace tanlang
