#ifndef __TAN_SRC_BASE_ERROR_H__
#define __TAN_SRC_BASE_ERROR_H__
#include "src/base/container.h"
#include "src/ast/fwd.h"

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno);
[[noreturn]] void __tan_abort();

namespace tanlang {

#ifdef DEBUG
#define TAN_ASSERT(expr) (static_cast<bool>((expr)) ?  \
  void (0) :                                           \
  __tan_assert_fail(#expr, __FILE__, __LINE__))
#else
#define TAN_ASSERT(expr)
#endif

// TODO: improve this
class Error {
public:
  explicit Error(const str &error_message);
  Error(const str &filename, const str &source, size_t line, size_t col, const str &error_message);
  Error(const str &filename, Token *token, const str &error_message);
  [[noreturn]] void print();

private:
  str _msg;
};

} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
