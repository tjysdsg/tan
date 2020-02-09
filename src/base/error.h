#ifndef __TAN_SRC_BASE_ERROR_H__
#define __TAN_SRC_BASE_ERROR_H__
#include <cassert>
#include <string>

namespace tanlang {

struct Token;

[[noreturn]] void report_code_error(const std::string &source,
                                    size_t line,
                                    size_t col,
                                    const std::string &error_message);
[[noreturn]] void report_code_error(Token *token, const std::string &error_message);

} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
