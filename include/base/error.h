#ifndef __TAN_SRC_BASE_ERROR_H__
#define __TAN_SRC_BASE_ERROR_H__
#include "base/container.h"
#include <stdexcept>

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno);

namespace tanlang {

#ifdef DEBUG
#define TAN_ASSERT(expr) (static_cast<bool>((expr)) ? void(0) : __tan_assert_fail(#expr, __FILE__, __LINE__))
#else
#define TAN_ASSERT(expr)
#endif

class CompileError : public std::runtime_error {
public:
  explicit CompileError(const str &msg);
  explicit CompileError(const char *msg);
};

class Token;

class Error {
public:
  explicit Error(const str &error_message);
  Error(const str &filename, const str &source, size_t line, size_t col, const str &error_message);
  Error(const str &filename, Token *token, const str &error_message);
  [[noreturn]] void raise() const;

private:
  str _msg;
};

} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
