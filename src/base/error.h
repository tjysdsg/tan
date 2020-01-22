#ifndef __TAN_SRC_BASE_ERROR_H__
#define __TAN_SRC_BASE_ERROR_H__

#include <cassert>
#include <string>

namespace tanlang {
void report_code_error(const std::string &source, size_t lineno, size_t column, const std::string &error_message);
void report_code_error(size_t l, size_t c, const std::string &error_message);
} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
