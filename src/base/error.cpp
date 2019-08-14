#include "src/base/error.h"
#include <iostream>

namespace tanlang {
    std::string operator*(std::string str, size_t num) {
        for (size_t i = 0; i < num; ++i) {
            str += str;
        }
        return str;
    }

    void handle_error(const std::string &msg) {
        fprintf(stderr, "%s", msg.c_str());
    }

    void report_code_error(const std::string &source, const size_t lineno,
                           const size_t column,
                           const std::string &error_message) {
        std::string error_output =
            "In line " + std::to_string(lineno) + ":\n" + source + "\n";
        error_output +=
            std::string(" ") * column + "^\n" + error_message + "\n";
        std::cerr << error_output;
    }
} // namespace tanlang
