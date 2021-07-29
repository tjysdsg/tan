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
