#include "src/common.h"

namespace tanlang {

bool is_ast_type_in(ASTNodeType t, const vector<ASTNodeType> &list) {
  return std::find(list.begin(), list.end(), t) != list.end();
}

bool is_string_in(const str &s, const vector<str> &list) {
  return std::find(list.begin(), list.end(), s) != list.end();
}

} // namespace tanlang

#ifdef DEBUG
const char *__asan_default_options() { return "detect_leaks=0"; }
#endif
