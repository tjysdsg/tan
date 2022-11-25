#ifndef TAN_LEXDEF_H
#define TAN_LEXDEF_H
#include "base.h"
#include <array>

namespace tanlang {

enum class TokenType {
  END = -1,    /// EOF
  COMMENTS,    ///
  KEYWORD,     ///
  INT,         ///
  FLOAT,       ///
  ID,          /// identifier
  CHAR,        /// character
  STRING,      /// string literal
  PUNCTUATION, /// , ; . ( ) { } etc.
  RELOP,       /// relational operator
  UOP,         /// unary operator
  BOP,         /// binary operator
};

extern umap<TokenType, str> token_type_names;
extern const vector<str> KEYWORDS;
extern const vector<char> PUNCTUATIONS;
extern const vector<str> TERMINAL_TOKENS;
extern const vector<str> ARITHMETIC_TOKENS;
// any symbol in OP can both be an operator itself or the start of an operator
extern const vector<char> OP;
extern const vector<str> OP_ALL;
extern umap<str, TokenType> OPERATION_VALUE_TYPE_MAP;

class SourceLoc {
public:
  SourceLoc() = delete;
  SourceLoc(size_t line, size_t col) : _line(line), _col(col) {}
  size_t get_line() const { return _line; }
  size_t get_col() const { return _col; }

private:
  size_t _line = 0;
  size_t _col = 0;
};

class Token {
public:
  Token() = delete;
  ~Token();
  Token(TokenType tokenType, size_t line, size_t col, str value, str source_line);
  [[nodiscard]] str to_string() const;
  std::ostream &operator<<(std::ostream &os) const;
  TokenType get_type() const;
  void set_type(TokenType type);
  const str &get_value() const;
  SourceLoc *get_loc() const;
  const str &get_source_line() const;
  bool is_unsigned() const;
  void set_is_unsigned(bool is_unsigned);
  size_t get_line() const;
  size_t get_col() const;

private:
  TokenType _type = TokenType::END;
  str _value;
  SourceLoc *_loc = nullptr;
  str _source_line;
  bool _is_unsigned = false;
};

} // namespace tanlang

#endif /*TAN_LEXDEF_H*/
