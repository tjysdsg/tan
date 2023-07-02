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

// TODO: start transitioning to categorized errors
// REMEMBER to add to Error::ERROR_TYPE_ENUM_TO_STRING
enum class ErrorType {
  GENERIC_ERROR,
  ASSERTION_FAILED,
  FILE_NOT_FOUND,
  SYNTAX_ERROR,
  SEMANTIC_ERROR,
  UNKNOWN_SYMBOL,

  NOT_IMPLEMENTED,
};

class Error;

class CompileException : public std::runtime_error {
public:
  CompileException(Error *err, const str &msg);
  CompileException(Error *err, const char *msg);

  [[nodiscard]] ErrorType type() const;

private:
  Error *_error = nullptr;
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
  [[deprecated]] Error(const str &filename, const str &source, size_t line, size_t col, const str &error_message);
  [[deprecated]] Error(const str &filename, Token *token, const str &error_message);
  [[noreturn]] void raise() const;

  [[nodiscard]] ErrorType type() const { return _type; }

private:
  str _msg;
  ErrorType _type = ErrorType::GENERIC_ERROR;
};

} // namespace tanlang

#endif // __TAN_SRC_BASE_ERROR_H__
