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
// any symbol in OP can both be an operator itself or the start of an operator
extern const vector<char> OP;
extern const vector<str> OP_ALL;
extern umap<str, TokenType> OPERATION_VALUE_TYPE_MAP;

class Token {
public:
  Token() = delete;
  ~Token() = default;
  Token(TokenType tokenType, uint32_t line, uint32_t col, str value, str source_line);
  TokenType get_type() const;
  void set_type(TokenType type);
  const str &get_value() const;
  str_view get_source_line() const;
  bool is_unsigned() const;
  void set_is_unsigned(bool is_unsigned);
  uint32_t get_line() const;
  uint32_t get_col() const;

private:
  TokenType _type = TokenType::END;
  str _value{};
  uint32_t _line = 0;
  uint32_t _col = 0;
  str_view _source_line{};
  bool _is_unsigned = false;
};

} // namespace tanlang

#endif /*TAN_LEXDEF_H*/
