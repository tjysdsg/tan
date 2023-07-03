#include "base/error.h"
#include "source_file/token.h"
#include "backtrace/tan_backtrace.h"
#include <fmt/core.h>

using namespace tanlang;

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno) {
  print_back_trace();
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

Error::Error(ErrorType type, SourceSpan span, const str &error_message) : _type(type) {
  SourceFile *src = span.src();
  _msg = fmt::format("[{}] in {}:{} {}\n", ERROR_TYPE_ENUM_TO_STRING[type], src->get_filename(), span.start().l,
                     error_message);

  uint32_t col = span.start().c;
  for (uint32_t line = span.start().l; line <= span.end().l; ++line) {
    str src_line = src->get_line(line);

    str indent = col > 0 ? str(col, ' ') : "";

    uint32_t end_col = (uint32_t)src_line.length(); // exclusive
    if (line == span.end().l) {
      end_col = span.end().c + 1;
    }
    str indicator = str(end_col - col, '^');

    _msg += fmt::format("{}\n{}{}\n", src_line, indent, indicator);

    col = 0;
  }
}

Error::Error(ErrorType type, Token *start, Token *end, const str &error_message)
    : Error(type, Token::GetSourceSpan(*start, *end), error_message) {}

void Error::raise() const { throw CompileException((Error *)this, _msg); }

ErrorType Error::type() const { return _type; }

#define ERROR_TYPE_TO_STRING_HELPER(x) \
  { ErrorType::x, #x }

umap<ErrorType, str> Error::ERROR_TYPE_ENUM_TO_STRING{
    ERROR_TYPE_TO_STRING_HELPER(GENERIC_ERROR),   ERROR_TYPE_TO_STRING_HELPER(ASSERTION_FAILED),
    ERROR_TYPE_TO_STRING_HELPER(FILE_NOT_FOUND),  ERROR_TYPE_TO_STRING_HELPER(SYNTAX_ERROR),
    ERROR_TYPE_TO_STRING_HELPER(NOT_IMPLEMENTED), ERROR_TYPE_TO_STRING_HELPER(SEMANTIC_ERROR),
    ERROR_TYPE_TO_STRING_HELPER(UNKNOWN_SYMBOL),  ERROR_TYPE_TO_STRING_HELPER(IMPORT_ERROR),
    ERROR_TYPE_TO_STRING_HELPER(TYPE_ERROR),      ERROR_TYPE_TO_STRING_HELPER(COMPILE_ERROR),
    ERROR_TYPE_TO_STRING_HELPER(LINK_ERROR),
};
