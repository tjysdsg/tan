#ifndef TAN_LEXDEF_H
#define TAN_LEXDEF_H
#include "reader.h"
#include <array>
#include <string>
#include <unordered_map>

namespace tanlang {

enum class TokenType {
  END = -1, COMMENTS, KEYWORD, INT, FLOAT, ID,          // identifier
  CHAR,        // character
  STRING,      // string literal
  PUNCTUATION, // , ; . ( ) { } etc.
  RELOP,       // relational operator
  UOP,         // unary operator
  BOP,         // binary operator
};

extern std::unordered_map<TokenType, std::string> token_type_names;
extern const std::vector<std::string> KEYWORDS;
extern const std::vector<char> PUNCTUATIONS;
// any symbol in OP can both be an operator itself or the start of an operator
extern const std::vector<char> OP;
extern const std::vector<std::string> OP_ALL;
extern std::unordered_map<std::string, TokenType> OPERATION_VALUE_TYPE_MAP;

struct Token {
  TokenType type = TokenType::END;
  std::string value = "";
  size_t l = 0, c = 0;
  line_info *line;

  Token() = default;

  Token(TokenType tokenType, std::string value, const code_ptr &cursor, line_info *line)
      : type(tokenType),
      value(std::move(value)),
      l(static_cast<size_t>(cursor.l)),
      c(static_cast<size_t>(cursor.c)),
      line(line) {}

  ~Token() = default;

  [[nodiscard]] std::string to_string() const;
  std::ostream &operator<<(std::ostream &os) const;
};

} // namespace tanlang

#endif /*TAN_LEXDEF_H*/
