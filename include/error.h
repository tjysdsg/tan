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

#ifdef DEBUG
#define ABORT() __tan_abort()
#else
#define ABORT() exit(1)
#endif

class ErrorCatcher {
public:
  friend class Error;
  using callback_t = std::function<void(const str &)>;

public:
  void register_callback(callback_t handler);

private:
  callback_t _callback;
};

// TODO: improve this
class Error {
public:
  static void CatchErrors(ErrorCatcher *handler_class);
  static void ResetErrorCatcher();

public:
  explicit Error(const str &error_message);
  Error(const str &filename, const str &source, size_t line, size_t col, const str &error_message);
  Error(const str &filename, Token *token, const str &error_message);
  [[noreturn]] void raise() const;

private:
  static inline ErrorCatcher *__catcher = nullptr;
  str _msg;
};

} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
