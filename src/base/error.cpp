#include "base.h"
#include "reader.h"
#include "token.h"
#include <iostream>
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>

static void print_back_trace() {
  unw_cursor_t cursor;
  unw_context_t context;
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    std::printf("0x%lx:", pc);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char *nameptr = sym;
      int status;
      char *demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }
      std::printf(" (%s+0x%lx)\n", nameptr, offset);
      std::free(demangled);
    } else {
      std::printf("unable to obtain symbol name for this frame\n");
    }
  }
}

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno) {
  std::cerr << "ASSERTION FAILED: " << expr << "\n";
  std::cerr << "at: " << file << ":" << std::to_string(lineno) << "\n";
  print_back_trace();
  abort();
}

namespace tanlang {

void report_code_error(const str &source, size_t line, size_t col, const str &error_message) {
  str error_output =
      "[ERROR] at LINE" + std::to_string(line) + ": " + error_message + "\n" + source + "\n" + str(col, ' ') + "^";
  throw std::runtime_error(error_output);
}

void report_code_error(Token *token, const str &error_message) {
  report_code_error(token->line->code, token->l, token->c, error_message);
}

} // namespace tanlang
