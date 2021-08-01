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

void Error::raise() const {
  if (Error::__catcher) {
    Error::__catcher->_callback(_msg);
  } else {
    std::cerr << _msg << '\n';
    ABORT();
  }
}

void ErrorCatcher::register_callback(callback_t handler) { _callback = handler; }

void Error::ResetErrorCatcher() { __catcher = nullptr; }

void Error::CatchErrors(ErrorCatcher *catcher) {
  if (!catcher) {
    std::cerr << "Invalid error catcher\n";
    print_back_trace();
    exit(1);
  }

  if (__catcher) {
    std::cerr << "Not allowed to have multiple error catchers\n";
    print_back_trace();
    exit(1);
  }

  __catcher = catcher;
}
