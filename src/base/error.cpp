#include "src/base/error.h"
#include <iostream>

namespace tanlang {
std::string operator*(std::string str, size_t num) {
  for (size_t i = 0; i < num; ++i) {
    str += str;
  }
  return str;
}

void report_code_error(const std::string &source, const size_t lineno, const size_t column,
                       const std::string &error_message) {
  std::string error_output = "In line " + std::to_string(lineno) + ":\n" + source + "\n";
  error_output += std::string(" ") * column + "^\n" + error_message + "\n";
  throw std::runtime_error(error_output);
}

void report_code_error(size_t l, size_t c, const std::string &error_message) {
  std::string
      error_output = "[ERROR] at LINE " + std::to_string(l) + ":COL " + std::to_string(c) + " " + error_message + '\n';
  throw std::runtime_error(error_output);
}
} // namespace tanlang
