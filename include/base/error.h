#ifndef __TAN_SRC_BASE_ERROR_H__
#define __TAN_SRC_BASE_ERROR_H__
#include "base/container.h"
#include "source_file/source_file.h"
#include <stdexcept>

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno);

namespace tanlang {

#ifdef DEBUG
#define TAN_ASSERT(expr) (static_cast<bool>((expr)) ? void(0) : __tan_assert_fail(#expr, __FILE__, __LINE__))
#else
#define TAN_ASSERT(expr)
#endif

// REMEMBER to add to Error::ERROR_TYPE_ENUM_TO_STRING
enum class ErrorType {
  GENERIC_ERROR,
  NOT_IMPLEMENTED,
  ASSERTION_FAILED,
  FILE_NOT_FOUND,
  SYNTAX_ERROR,
  SEMANTIC_ERROR,
  UNKNOWN_SYMBOL,
  IMPORT_ERROR,
  TYPE_ERROR,
  COMPILE_ERROR,
  LINK_ERROR,
};

class Error;

class CompileException : public std::runtime_error {
public:
  CompileException(ErrorType error_type, const str &msg);
  CompileException(ErrorType error_type, const char *msg);

  [[nodiscard]] ErrorType type() const;

private:
  ErrorType _type = ErrorType::GENERIC_ERROR;
};

class Token;

class Error {
public:
  static umap<ErrorType, str> ERROR_TYPE_ENUM_TO_STRING;

public:
  explicit Error(const str &error_message);
  Error(ErrorType type, const str &error_message);
  Error(ErrorType type, Token *start, Token *end, const str &error_message);
  Error(ErrorType type, SourceSpan span, const str &error_message);

  [[noreturn]] void raise() const;
  [[nodiscard]] ErrorType type() const;

private:
  str _msg;
  ErrorType _type = ErrorType::GENERIC_ERROR;
};

} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
