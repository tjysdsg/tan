#include "base/error.h"
#include "lexer/token.h"
#include <fmt/core.h>

using namespace tanlang;

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno) {
  throw CompileError(fmt::format("ASSERTION FAILED: {}\nat {}:{}\n", expr, file, std::to_string(lineno)));
}

CompileError::CompileError(const str &msg) : std::runtime_error(msg) {}

CompileError::CompileError(const char *msg) : std::runtime_error(msg) {}

Error::Error(const str &error_message) { _msg = "[ERROR] " + error_message; }

Error::Error(const str &filename, const str &source, size_t line, size_t col, const str &error_message) {
  str indent = col > 0 ? str(col - 1, ' ') : "";
  _msg = fmt::format("[ERROR] at {}:{} {}\n{}\n{}^", filename, line, error_message, source, indent);
}

Error::Error(const str &filename, Token *token, const str &error_message) {
  str indent = token->get_col() > 0 ? str(token->get_col() - 1, ' ') : "";
  _msg = fmt::format("[ERROR] at {}:{} {}\n{}\n{}^", filename, token->get_line() + 1, error_message,
                     token->get_source_line(), indent);
}

void Error::raise() const { throw CompileError(_msg); }
