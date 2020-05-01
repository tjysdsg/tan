#ifndef __TAN_SRC_BASE_ERROR_H__
#define __TAN_SRC_BASE_ERROR_H__
#include <string>

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno);

namespace tanlang {

struct Token;

#define TAN_ASSERT(expr) (static_cast<bool>((expr)) ?  \
  void (0) :                                           \
  __tan_assert_fail(#expr, __FILE__, __LINE__))

[[noreturn]] void report_code_error(const std::string &source,
    size_t line,
    size_t col,
    const std::string &error_message);
[[noreturn]] void report_code_error(Token *token, const std::string &error_message);

} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
