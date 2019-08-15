#ifndef __TAN_SRC_BASE_ERROR_H__
#define __TAN_SRC_BASE_ERROR_H__
#include <cassert>
#include <string>
namespace tanlang {
    void handle_error(const std::string &msg);
	void report_code_error(const std::string &source, const size_t lineno, const size_t column,
	                       const std::string &error_message);
} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
