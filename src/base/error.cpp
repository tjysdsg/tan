#include "base/error.h"
#include "lexer/token.h"
#include <fmt/core.h>

using namespace tanlang;

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno) {
  Error(ErrorType::ASSERTION_FAILED,
        fmt::format("ASSERTION FAILED: {}\nat {}:{}\n", expr, file, std::to_string(lineno)))
      .raise();
}

CompileException::CompileException(Error *err, const str &msg) : std::runtime_error(msg), _error(err) {}

CompileException::CompileException(Error *err, const char *msg) : std::runtime_error(msg), _error(err) {}

ErrorType CompileException::type() const {
  if (_error) {
    return _error->type();
  }

  return ErrorType::GENERIC_ERROR;
}

Error::Error(const str &error_message) { _msg = "[ERROR] " + error_message; }

Error::Error(ErrorType type, const str &error_message) : _type(type) {
  _msg = fmt::format("[{}] {}", ERROR_TYPE_ENUM_TO_STRING[type], error_message);
}

Error::Error(const str &filename, const str &source, size_t line, size_t col, const str &error_message)
    : _type(ErrorType::GENERIC_ERROR) {
  str indent = col > 0 ? str(col - 1, ' ') : "";
  _msg = fmt::format("[GENERIC_ERROR] at {}:{} {}\n{}\n{}^", filename, line, error_message, source, indent);
}

Error::Error(const str &filename, Token *token, const str &error_message) : _type(ErrorType::GENERIC_ERROR) {
  str indent = token->get_col() > 0 ? str(token->get_col() - 1, ' ') : "";
  _msg = fmt::format("[GENERIC_ERROR] at {}:{} {}\n{}\n{}^", filename, token->get_line() + 1, error_message,
                     token->get_source_line(), indent);
}

void Error::raise() const { throw CompileException((Error *)this, _msg); }

#define ERROR_TYPE_TO_STRING_HELPER(x) \
  { ErrorType::x, #x }

umap<ErrorType, str> Error::ERROR_TYPE_ENUM_TO_STRING{
    ERROR_TYPE_TO_STRING_HELPER(GENERIC_ERROR),
    ERROR_TYPE_TO_STRING_HELPER(ASSERTION_FAILED),
    ERROR_TYPE_TO_STRING_HELPER(FILE_NOT_FOUND),
};
