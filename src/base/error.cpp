#include "src/base/error.h"
#include "token.h"
#include <iostream>

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno) {
  std::cerr << "ASSERTION FAILED: " << expr << "\n";
  std::cerr << "at: " << file << ":" << std::to_string(lineno) << "\n";
  abort();
}

namespace tanlang {

void report_code_error(const std::string &source, size_t line, size_t col, const std::string &error_message) {
  std::string error_output =
      "[ERROR] at LINE" + std::to_string(line) + ": " + error_message + "\n" + source + "\n" + std::string(col, ' ')
          + "^";
  throw std::runtime_error(error_output);
}

void report_code_error(Token *token, const std::string &error_message) {
  report_code_error(token->line->code, token->l, token->c, error_message);
}

} // namespace tanlang
