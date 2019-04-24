#ifndef __TAN_SRC_BASE_ERROR_H__
#define __TAN_SRC_BASE_ERROR_H__
#include <string>
#include <cassert>

namespace tanlang {
    void report_error(const std::string &source, const size_t lineno,
                      const size_t column, const std::string &error_message);
}

#endif // __TAN_SRC_BASE_ERROR_H__
