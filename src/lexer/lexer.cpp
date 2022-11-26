#include "lexer/lexer.h"
#include "base.h"
#include "lexer/token.h"
#include "lexer/reader.h"
#include <algorithm>
#include <cctype>
#include <regex>

namespace tanlang {

#define IS_DELIMITER(x)                                                                                               \
  (x == ';' || std::isspace(x) || x == ',' || x == '.' || x == '!' || x == '@' || x == '#' || x == '$' || x == '%' || \
   x == '^' || x == '&' || x == '*' || x == '(' || x == ')' || x == '-' || x == '+' || x == '=' || x == ';' ||        \
   x == '<' || x == '>' || x == '/' || x == '?' || x == '\\' || x == '|' || x == '{' || x == '}' || x == '[' ||       \
   x == ']' || x == '\'' || x == '"' || x == ':')

[[noreturn]] static void report_error(Reader *reader, Cursor c, const str &message) {
  Error err(reader->get_filename(), reader->get_line(c.l), c.l + 1, c.c + 1, message);
  err.raise();
}

Cursor skip_whitespace(Reader *reader, Cursor ptr) {
  const auto end = reader->end();
  while (ptr < end && (std::isspace(*ptr) || *ptr == '\0')) {
    ++ptr;
  }
  return ptr;
}

/**
 * \note: For all tokenize_xx functions
 *      @start is at least one token before the end
 * \note: Call of tokenize_keyword must before that of
 *      tokenize_id
 */
Token *tokenize_id(Reader *reader, Cursor &start) {
  Token *ret = nullptr;
  auto forward = start;
  const auto end = reader->end();
  while (forward != end) {
    if (std::isalnum(*forward) || *forward == '_') {
      ++forward;
    } else if (start == forward) {
      return nullptr;
    } else {
      ret = new Token(TokenType::ID, start.l, start.c, reader->substr(start, forward), reader->get_line(start.l));
      break;
    }
  }
  start = forward;
  return ret;
}

Token *tokenize_keyword(Reader *reader, Cursor &start) {
  // find whether the value is in KEYWORDS (in token.h/token.cpp) based on
  // returned value of tokenize_id()
  Cursor forward = start;
  auto *t = tokenize_id(reader, forward);
  if (t) {
    if (std::find(KEYWORDS.begin(), KEYWORDS.end(), t->get_value()) != KEYWORDS.end()) {
      t->set_type(TokenType::KEYWORD);
      start = forward;
    } else {
      delete (t);
      t = nullptr;
    }
  }
  return t;
}

Token *tokenize_comments(Reader *reader, Cursor &start) {
  Token *t = nullptr;
  auto next = reader->forward(start);
  if (*next == '/') { /// line comments
    auto value = reader->substr(reader->forward(next));
    t = new Token(TokenType::COMMENTS, start.l, start.c, value, reader->get_line(start.l));
    start.c = (uint32_t)reader->get_line(start.l).length();
    ++start;
  } else if (*next == '*') {                      /// block comments
    auto forward = start = reader->forward(next); /// forward now points to the character after "/*"

    /// trying to find "*/"
    while ((size_t)forward.l < reader->size()) {
      auto re = std::regex(R"(\*\/)");
      auto s = reader->get_line(forward.l);
      std::smatch result;
      if (std::regex_search(s, result, re)) {
        forward.c = (uint32_t)result.position(0); // forward is the position of */
        str comment_val = reader->substr(start, forward);
        t = new Token(TokenType::COMMENTS, start.l, start.c, comment_val, reader->get_line(start.l));
        forward.c += 2;
        start = forward;
        break;
      }
      ++forward.l;
      forward.c = 0;
    }
    if (!t) {
      report_error(reader, start, "Invalid comments");
    }
  } else {
    report_error(reader, start, "Invalid comments");
  }
  return t;
}

Token *tokenize_number(Reader *reader, Cursor &start) {
  auto forward = start;
  const auto end = reader->end();
  bool is_float = false;
  bool is_unsigned = false;
  bool contains_digit = false;
  auto start_digit_i = start;
  while (forward < end) {
    const char ch = *forward;
    if (std::isdigit(ch)) {
      contains_digit = true;
    } else if (*start_digit_i == '0' && contains_digit &&
               ((ch <= 'F' && ch >= 'A') || (ch <= 'f' && ch >= 'a') || ch == 'x' || ch == 'X')) {
    } else if (contains_digit && !is_float && ch == 'u') { /// explicitly unsigned
      is_unsigned = true;
    } else if (contains_digit && ch == '.') {
      is_float = true;
    } else if (IS_DELIMITER(ch)) {
      break;
    } else {
      report_error(reader, forward, "Unexpected character within a number literal");
    }
    ++forward;
  }

  auto *t = new Token(is_float ? TokenType::FLOAT : TokenType::INT, start.l, start.c, reader->substr(start, forward),
                      reader->get_line(start.l));
  t->set_is_unsigned(is_unsigned);
  start = forward;
  return t;
}

char escape_char(char c) {
  /// https://en.cppreference.com/w/cpp/language/escape
  switch (c) {
  case '\'':
    return '\'';
  case '\"':
    return '\"';
  case '\\':
    return '\\';
  case '?':
    return '\?';
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 'f':
    return '\f';
  case 'n':
    return '\n';
  case 'r':
    return '\r';
  case 't':
    return '\t';
  case 'v':
    return '\v';
  default:
    return -1;
  }
}

Token *tokenize_char(Reader *reader, Cursor &start) {
  Token *t = nullptr;
  auto forward = reader->forward(start);
  const auto end = reader->end();

  while (forward < end && *forward != '\'') {
    if (*forward == '\\') {
      ++forward;
    }
    ++forward;
  }

  if (end <= forward) {
    auto lineno = reader->size() - 1;
    auto src = reader->get_line(lineno);
    Error err(reader->get_filename(), src, lineno, src.length() - 1, "Incomplete character literal");
    err.raise();
  } else {
    str value = reader->substr(reader->forward(start), forward); // not including the single quotes
    if (value[0] == '\\') {
      if (value.length() != 2) {
        report_error(reader, forward, "Invalid character literal");
      }
      value = str(1, escape_char(value[1]));
    } else if (value.length() != 1) {
      report_error(reader, forward, "Invalid character literal");
    }
    t = new Token(TokenType::CHAR, start.l, start.c, value, reader->get_line(start.l));
    start = reader->forward(forward);
  }
  return t;
}

Token *tokenize_string(Reader *reader, Cursor &start) {
  Token *t = nullptr;
  auto forward = reader->forward(start);
  const auto end = reader->end();

  while (forward < end && *forward != '"') {
    if (*forward == '\\') { // escape
      ++forward;
    }
    ++forward;
  }

  if (end <= forward) {
    auto lineno = reader->size() - 1;
    auto src = reader->get_line(lineno);
    Error err(reader->get_filename(), src, lineno, src.length() - 1, "Incomplete string literal");
    err.raise();
  } else {
    str value = reader->substr(reader->forward(start), forward); // not including the double quotes
    str escaped = "";
    size_t l = value.length();
    size_t start_i = 0;
    size_t i = 0;
    while (i < l) {
      char c = value[i];
      if (c == '\\') {
        escaped += value.substr(start_i, i - start_i);
        escaped += escape_char(value[i + 1]);
        start_i = i + 2;
        ++i;
      }
      ++i;
    }
    escaped += value.substr(start_i, l - start_i);
    t = new Token(TokenType::STRING, start.l, start.c, escaped, reader->get_line(start.l));
    start = (*reader).forward(forward);
  }
  return t;
}

Token *tokenize_punctuation(Reader *reader, Cursor &start) {
  Token *t = nullptr;
  auto next = reader->forward(start);
  size_t lineno = start.l;

  if (*start == '/' && (*next == '/' || *next == '*')) { /// line comment or block comment
    t = tokenize_comments(reader, start);
  } else if (*start == '\'') { /// char literal
    t = tokenize_char(reader, start);
  } else if (*start == '"') { /// string literal
    t = tokenize_string(reader, start);
  } else if (std::find(OP.begin(), OP.end(), *start) != OP.end()) { /// operators
    str value;
    {
      Cursor nnext = reader->forward(next);
      Cursor nnnext = reader->forward(nnext);
      Cursor back_ptr = reader->end();
      str two = reader->substr(start, nnext);
      str three = reader->substr(start, reader->forward(nnext));

      if (next < back_ptr && nnext < back_ptr &&
          std::find(OP_ALL.begin(), OP_ALL.end(), three) != OP_ALL.end()) { /// operator containing three characters
        value = reader->substr(start, nnnext);
        start = nnnext;
      } else if (next < back_ptr && std::find(OP_ALL.begin(), OP_ALL.end(), two) != OP_ALL.end()) {
        value = reader->substr(start, nnext);
        if (OPERATION_VALUE_TYPE_MAP.find(value) != OPERATION_VALUE_TYPE_MAP.end()) { /// operator containing two chars
          start = nnext;
        }
      } else {
        /// operator containing one chars
        value = str{*start};
        TAN_ASSERT(OPERATION_VALUE_TYPE_MAP.find(value) != OPERATION_VALUE_TYPE_MAP.end());
        start = next;
      }
    }
    // create new token, fill in token
    TokenType type = OPERATION_VALUE_TYPE_MAP[value];
    t = new Token(type, start.l, start.c, value, reader->get_line(lineno));
  } /// other punctuations
  else if (std::find(PUNCTUATIONS.begin(), PUNCTUATIONS.end(), *start) != PUNCTUATIONS.end()) {
    t = new Token(TokenType::PUNCTUATION, start.l, start.c, str(1, *start), reader->get_line(lineno));
    start = next;
  } else {
    t = nullptr;
  }
  return t;
}

vector<Token *> tokenize(Reader *reader) {
  Cursor start = reader->begin();
  if (reader->size() == 0) {
    return {};
  }
  vector<Token *> tokens;
  const auto end = reader->end();
  while (start < end) {
    /// if start with a letter
    if (std::isalpha(*start)) {
      auto *new_token = tokenize_keyword(reader, start);
      if (!new_token) {
        /// if this is not a keyword, probably an identifier
        new_token = tokenize_id(reader, start);
        if (!new_token) {
          report_error(reader, start, "Invalid identifier");
        }
      }
      tokens.emplace_back(new_token);
    } else if (*start == '_') {
      /// start with an underscore, must be an identifier
      auto *new_token = tokenize_id(reader, start);
      if (!new_token) {
        report_error(reader, start, "Invalid identifier");
      }
      tokens.emplace_back(new_token);
    } else if (std::isdigit(*start)) {
      /// number literal
      auto *new_token = tokenize_number(reader, start);
      if (!new_token) {
        report_error(reader, start, "Invalid number literal");
      }
      tokens.emplace_back(new_token);
    } else if (std::find(PUNCTUATIONS.begin(), PUNCTUATIONS.end(), *start) != PUNCTUATIONS.end()) {
      /// punctuations
      auto *new_token = tokenize_punctuation(reader, start);
      if (!new_token) {
        report_error(reader, start, "Invalid symbol(s)");
      }
      tokens.emplace_back(new_token);
    } else {
      report_error(reader, start, "Invalid symbol(s)");
    }
    start = skip_whitespace(reader, start);
  }
  return tokens;
}

} // namespace tanlang
