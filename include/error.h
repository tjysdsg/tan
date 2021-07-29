#ifndef __TAN_SRC_BASE_ERROR_H__
#define __TAN_SRC_BASE_ERROR_H__
#include "src/base/container.h"
#include "src/ast/fwd.h"
#include "token.h"
#include <fmt/core.h>
#include <iostream>

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

// TODO: improve this
class Error {
public:
  explicit Error(const str &error_message) {
    _msg = "[ERROR] " + error_message;
  }

  Error(const str &filename, const str &source, size_t line, size_t col, const str &error_message) {
    str indent = col > 0 ? str(col - 1, ' ') : "";
    _msg = fmt::format("[ERROR] at {}:{} {}\n{}\n{}^", filename, line, error_message, source, indent);
  }

  Error(const str &filename, Token *token, const str &error_message) {
    str indent = token->get_col() > 0 ? str(token->get_col() - 1, ' ') : "";
    _msg = fmt::format("[ERROR] at {}:{} {}\n{}\n{}^",
        filename,
        token->get_line() + 1,
        error_message,
        token->get_source_line(),
        indent);
  }

  void print() {
    std::cerr << _msg << '\n';
    ABORT();
  }

private:
  str _msg;
};

} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
