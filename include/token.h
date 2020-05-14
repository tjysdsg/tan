#ifndef TAN_LEXDEF_H
#define TAN_LEXDEF_H
#include "base.h"
#include <array>

namespace tanlang {

struct cursor;
struct line_info;

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

extern std::unordered_map<TokenType, str> token_type_names;
extern const vector<str> KEYWORDS;
extern const vector<char> PUNCTUATIONS;
// any symbol in OP can both be an operator itself or the start of an operator
extern const vector<char> OP;
extern const vector<str> OP_ALL;
extern std::unordered_map<str, TokenType> OPERATION_VALUE_TYPE_MAP;

struct Token {
  TokenType type = TokenType::END;
  str value = "";
  size_t l = 0, c = 0;
  line_info *line = nullptr;
  bool is_unsigned = false;

  Token() = default;
  ~Token() = default;
  Token(TokenType tokenType, str value, const cursor &cursor, const line_info *line);
  [[nodiscard]] str to_string() const;
  std::ostream &operator<<(std::ostream &os) const;
};

} // namespace tanlang

#endif /*TAN_LEXDEF_H*/
