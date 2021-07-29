#include "error.h"
#include "token.h"
#include <fmt/core.h>
#include <iostream>
#include "src/backtrace/tan_backtrace.h"

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno) {
  std::cerr << "ASSERTION FAILED: " << expr << "\n";
  std::cerr << "at: " << file << ":" << std::to_string(lineno) << "\n";
  print_back_trace();
  abort();
}

[[noreturn]] void __tan_abort() {
  print_back_trace();
  abort();
}

#ifdef DEBUG
#define ABORT() __tan_abort()
#else
#define ABORT() exit(1)
#endif

using namespace tanlang;

Error::Error(const str &error_message) {
  _msg = "[ERROR] " + error_message;
}

Error::Error(const str &filename, const str &source, size_t line, size_t col, const str &error_message) {
  str indent = col > 0 ? str(col - 1, ' ') : "";
  _msg = fmt::format("[ERROR] at {}:{} {}\n{}\n{}^", filename, line, error_message, source, indent);
}

Error::Error(const str &filename, Token *token, const str &error_message) {
  str indent = token->get_col() > 0 ? str(token->get_col() - 1, ' ') : "";
  _msg = fmt::format("[ERROR] at {}:{} {}\n{}\n{}^",
      filename,
      token->get_line() + 1,
      error_message,
      token->get_source_line(),
      indent);
}

void Error::print() {
  std::cerr << _msg << '\n';
  ABORT();
}
