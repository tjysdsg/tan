#include "lexer/lexer.h"
#include "base.h"
#include "source_file/token.h"
#include "source_file/source_file.h"
#include <algorithm>
#include <cctype>
#include <regex>

namespace tanlang {

#define IS_DELIMITER(x)                                                                                               \
  (x == ';' || std::isspace(x) || x == ',' || x == '.' || x == '!' || x == '@' || x == '#' || x == '$' || x == '%' || \
   x == '^' || x == '&' || x == '*' || x == '(' || x == ')' || x == '-' || x == '+' || x == '=' || x == ';' ||        \
   x == '<' || x == '>' || x == '/' || x == '?' || x == '\\' || x == '|' || x == '{' || x == '}' || x == '[' ||       \
   x == ']' || x == '\'' || x == '"' || x == ':')

[[noreturn]] static void report_error(SrcLoc c, const str &message) {
  // need to allocate memory otherwise this gets optimized out
  auto *err = new Error(ErrorType::SYNTAX_ERROR, SourceSpan(c, c), message);
  err->raise();
}

SrcLoc skip_whitespace(SourceFile *src, SrcLoc ptr) {
  const auto end = src->end();
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
Token *tokenize_id(SourceFile *src, SrcLoc &start) {
  Token *ret = nullptr;
  auto forward = start;
  const auto end = src->end();
  while (forward != end) {
    if (std::isalnum(*forward) || *forward == '_') {
      ++forward;
    } else if (start == forward) {
      return nullptr;
    } else {
      ret = new Token(TokenType::ID, start.l, start.c, src->substr(start, forward), src);
      break;
    }
  }
  start = forward;
  return ret;
}

Token *tokenize_keyword(SourceFile *src, SrcLoc &start) {
  // find whether the value is in KEYWORDS (in token.h/token.cpp) based on
  // returned value of tokenize_id()
  SrcLoc forward = start;
  auto *t = tokenize_id(src, forward);
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

Token *tokenize_comments(SourceFile *src, SrcLoc &start) {
  Token *t = nullptr;
  auto next = src->forward(start);
  if (*next == '/') { /// line comments
    auto value = src->substr(src->forward(next));
    t = new Token(TokenType::COMMENTS, start.l, start.c, value, src);
    start.c = (uint32_t)src->get_line(start.l).length();
    ++start;
  } else if (*next == '*') {                   /// block comments
    auto forward = start = src->forward(next); /// forward now points to the character after "/*"

    /// trying to find "*/"
    while ((size_t)forward.l < src->size()) {
      auto re = std::regex(R"(\*\/)");
      auto s = src->get_line(forward.l);
      std::smatch result;
      if (std::regex_search(s, result, re)) {
        forward.c = (uint32_t)result.position(0); // forward is the position of */
        str comment_val = src->substr(start, forward);
        t = new Token(TokenType::COMMENTS, start.l, start.c, comment_val, src);
        forward.c += 2;
        start = forward;
        break;
      }
      ++forward.l;
      forward.c = 0;
    }
    if (!t) {
      report_error(start, "Invalid comments");
    }
  } else {
    report_error(start, "Invalid comments");
  }
  return t;
}

// we don't care about if the number literal is actually correct
Token *tokenize_number(SourceFile *src, SrcLoc &start) {
  auto forward = start;
  const auto end = src->end();
  bool is_float = false;
  bool is_unsigned = false;
  bool contains_digit = false;
  auto start_digit_i = start;
  while (forward < end) {
    const char ch = *forward;

    if (std::isdigit(ch)) {
      contains_digit = true;

    } else if (*start_digit_i == '0' &&
               ((ch <= 'F' && ch >= 'A') || (ch <= 'f' && ch >= 'a') || ch == 'x' || ch == 'X')) {

    } else if (contains_digit && !is_float && ch == 'u') { // explicitly unsigned
      is_unsigned = true;

    } else if (contains_digit && ch == '.') {
      is_float = true;

    } else if (IS_DELIMITER(ch)) {
      break;

    } else {
      report_error(forward, "Unexpected character within a number literal");
    }

    ++forward;
  }

  auto *t = new Token(is_float ? TokenType::FLOAT : TokenType::INT, start.l, start.c, src->substr(start, forward), src);
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

Token *tokenize_char(SourceFile *src, SrcLoc &start) {
  Token *t = nullptr;
  auto forward = src->forward(start);
  const auto end = src->end();

  while (forward < end && *forward != '\'') {
    if (*forward == '\\') {
      ++forward;
    }
    ++forward;
  }

  if (end <= forward) {
    auto lineno = src->size() - 1;
    auto line = src->get_line(lineno);
    Error(ErrorType::SYNTAX_ERROR, SourceSpan(src, (uint32_t)lineno, (uint32_t)line.length() - 1),
          "Incomplete character literal")
        .raise();
  } else {
    str value = src->substr(src->forward(start), forward); // not including the single quotes
    if (value[0] == '\\') {
      if (value.length() != 2) {
        report_error(forward, "Invalid character literal");
      }
      value = str(1, escape_char(value[1]));
    } else if (value.length() != 1) {
      report_error(forward, "Invalid character literal");
    }
    t = new Token(TokenType::CHAR, start.l, start.c, value, src);
    start = src->forward(forward);
  }
  return t;
}

Token *tokenize_string(SourceFile *src, SrcLoc &start) {
  Token *t = nullptr;
  auto forward = src->forward(start);
  const auto end = src->end();

  while (forward < end && *forward != '"') {
    if (*forward == '\\') { // escape
      ++forward;
    }
    ++forward;
  }

  if (end <= forward) {
    auto lineno = src->size() - 1;
    auto line = src->get_line(lineno);
    Error(ErrorType::SYNTAX_ERROR, SourceSpan(src, (uint32_t)lineno, (uint32_t)line.length() - 1),
          "Incomplete string literal")
        .raise();
  } else {
    str value = src->substr(src->forward(start), forward); // not including the double quotes
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
    t = new Token(TokenType::STRING, start.l, start.c, escaped, src);
    start = (*src).forward(forward);
  }
  return t;
}

Token *tokenize_punctuation(SourceFile *src, SrcLoc &start) {
  Token *t = nullptr;
  auto next = src->forward(start);

  if (*start == '/' && (*next == '/' || *next == '*')) {            /// line comment or block comment
    t = tokenize_comments(src, start);
  } else if (*start == '\'') {                                      /// char literal
    t = tokenize_char(src, start);
  } else if (*start == '"') {                                       /// string literal
    t = tokenize_string(src, start);
  } else if (std::find(OP.begin(), OP.end(), *start) != OP.end()) { /// operators
    str value;
    {
      SrcLoc nnext = src->forward(next);
      SrcLoc nnnext = src->forward(nnext);
      SrcLoc back_ptr = src->end();
      str two = src->substr(start, nnext);
      str three = src->substr(start, src->forward(nnext));

      if (next < back_ptr && nnext < back_ptr &&
          std::find(OP_ALL.begin(), OP_ALL.end(), three) != OP_ALL.end()) { /// operator containing three characters
        value = src->substr(start, nnnext);
        start = nnnext;
      } else if (next < back_ptr && std::find(OP_ALL.begin(), OP_ALL.end(), two) != OP_ALL.end()) {
        value = src->substr(start, nnext);
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
    t = new Token(type, start.l, start.c, value, src);
  } /// other punctuations
  else if (std::find(PUNCTUATIONS.begin(), PUNCTUATIONS.end(), *start) != PUNCTUATIONS.end()) {
    t = new Token(TokenType::PUNCTUATION, start.l, start.c, str(1, *start), src);
    start = next;
  } else {
    t = nullptr;
  }
  return t;
}

vector<Token *> tokenize(SourceFile *src) {
  SrcLoc start = src->begin();
  if (src->size() == 0) {
    return {};
  }
  vector<Token *> tokens;
  const auto end = src->end();
  while (start < end) {
    /// if start with a letter
    if (std::isalpha(*start)) {
      auto *new_token = tokenize_keyword(src, start);
      if (!new_token) {
        /// if this is not a keyword, probably an identifier
        new_token = tokenize_id(src, start);
        if (!new_token) {
          report_error(start, "Invalid identifier");
        }
      }
      tokens.emplace_back(new_token);
    } else if (*start == '_') {
      /// start with an underscore, must be an identifier
      auto *new_token = tokenize_id(src, start);
      if (!new_token) {
        report_error(start, "Invalid identifier");
      }
      tokens.emplace_back(new_token);
    } else if (std::isdigit(*start)) {
      /// number literal
      auto *new_token = tokenize_number(src, start);
      if (!new_token) {
        report_error(start, "Invalid number literal");
      }
      tokens.emplace_back(new_token);
    } else if (std::find(PUNCTUATIONS.begin(), PUNCTUATIONS.end(), *start) != PUNCTUATIONS.end()) {
      /// punctuations
      auto *new_token = tokenize_punctuation(src, start);
      if (!new_token) {
        report_error(start, "Invalid symbol(s)");
      }
      tokens.emplace_back(new_token);
    } else {
      report_error(start, "Invalid symbol(s)");
    }
    start = skip_whitespace(src, start);
  }
  return tokens;
}

} // namespace tanlang
