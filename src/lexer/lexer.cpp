#include "lexer.h"
#include "base.h"
#include "token.h"
#include <algorithm>
#include <cctype>
#include <regex>

namespace tanlang {

#define IS_DELIMITER(x)                                                        \
  (x == ';' || std::isspace(x) || x == ',' || x == '.' || x == '!' ||          \
   x == '@' || x == '#' || x == '$' || x == '%' || x == '^' || x == '&' ||     \
   x == '*' || x == '(' || x == ')' || x == '-' || x == '+' || x == '=' ||     \
   x == ';' || x == '<' || x == '>' || x == '/' || x == '?' || x == '\\' ||    \
   x == '|' || x == '{' || x == '}' || x == '[' || x == ']' || x == '\'' ||    \
   x == '"' || x == ':')

code_ptr skip_whitespace(Reader *reader, code_ptr ptr) {
  const auto end = reader->back_ptr();
  while (ptr < end && (std::isspace((*reader)[ptr]) || (*reader)[ptr] == '\0')) {
    ptr = reader->forward_ptr(ptr);
  }
  return ptr;
}

/**
 * \note: For all tokenize_xx functions
 *      @start is at least one token before the end
 * \note: Call of tokenize_keyword must before that of
 *      tokenize_id
 */
Token *tokenize_id(Reader *reader, code_ptr &start) {
  Token *ret = nullptr;
  auto forward = start;
  const auto end = reader->back_ptr();
  while (forward != end) {
    if (std::isalnum((*reader)[forward]) || (*reader)[forward] == '_') {
      forward = reader->forward_ptr(forward);
    } else if (start == forward) {
      return nullptr;
    } else {
      ret = new Token(TokenType::ID, (*reader)(start, forward), start, &(*reader)[static_cast<size_t>(start.l)]);
      break;
    }
  }
  start = forward;
  return ret;
}

Token *tokenize_keyword(Reader *reader, code_ptr &start) {
  // find whether the value is in KEYWORDS (in token.h/token.cpp) based on
  // returned value of tokenize_id()
  code_ptr forward = start;
  auto *t = tokenize_id(reader, forward);
  if (t) {
    if (std::find(KEYWORDS.begin(), KEYWORDS.end(), t->value) != KEYWORDS.end()) {
      t->type = TokenType::KEYWORD;
      start = forward;
    } else {
      delete (t);
      t = nullptr;
    }
  }
  return t;
}

Token *tokenize_comments(Reader *reader, code_ptr &start) {
  Token *t = nullptr;
  auto next = reader->forward_ptr(start);
  if ((*reader)[next] == '/') {
    // line comments
    auto value = (*reader)(reader->forward_ptr(next));
    t = new Token(TokenType::COMMENTS, value, start, &(*reader)[static_cast<size_t>(start.l)]);
    start.c = static_cast<long>((*reader)[static_cast<size_t>(start.l)].code.length());
    start = (*reader).forward_ptr(start);
  } else if ((*reader)[next] == '*') {
    /* block comments */
    auto forward = start;
    // loop for each line
    while (static_cast<size_t>(forward.l) < reader->size()) {
      auto re = std::regex(R"(.*\*\/)");
      auto str = (*reader)(forward);
      std::smatch result;
      if (std::regex_match(str, result, re)) {
        std::string value = str.substr(2, static_cast<size_t>(result.length(0) - 4));
        t = new Token(TokenType::COMMENTS, value, start, &(*reader)[static_cast<size_t>(start.l)]);
        forward.c = result.length(0);
        start = forward;
      }
      ++forward.l;
    }
    if (!t) {
      report_code_error((*reader)[static_cast<size_t>(start.l)].code,
          static_cast<size_t>(start.l),
          static_cast<size_t>(start.c),
          "Invalid comments");
    }
  } else {
    report_code_error((*reader)[static_cast<size_t>(start.l)].code,
        static_cast<size_t>(start.l),
        static_cast<size_t>(start.c),
        "Invalid comments");
  }
  return t;
}

Token *tokenize_number(Reader *reader, code_ptr &start) {
  auto forward = start;
  const auto end = reader->back_ptr();
  bool is_float = false;
  bool is_unsigned = false;
  auto *t = new Token;
  bool contains_digit = false;
  auto start_digit_i = start;
  while (forward < end) {
    const char ch = (*reader)[forward];
    if (std::isdigit(ch)) {
      contains_digit = true;
    } else if ((*reader)[start_digit_i] == '0' && contains_digit
        && ((ch <= 'F' && ch >= 'A') || (ch <= 'f' && ch >= 'a') || ch == 'x' || ch == 'X')) {
    } else if (contains_digit && !is_float && ch == 'u') { /// explicitly unsigned
      is_unsigned = true;
    } else if (contains_digit && ch == '.') {
      is_float = true;
    } else if (IS_DELIMITER(ch)) {
      break;
    } else {
      report_code_error((*reader)[(size_t) forward.l].code,
          static_cast<size_t>(forward.l),
          static_cast<size_t>(forward.c),
          "Unexpected character within a number literal");
      delete t;
      return nullptr;
    }
    forward = (*reader).forward_ptr(forward);
  }
  t->type = is_float ? TokenType::FLOAT : TokenType::INT;
  t->value = (*reader)(start, forward);
  t->l = static_cast<size_t>(start.l);
  t->c = static_cast<size_t>(start.c);
  t->is_unsigned = is_unsigned;
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

// TODO: check line breaks inside two quotation marks
Token *tokenize_char(Reader *reader, code_ptr &start) {
  Token *t = nullptr;
  auto forward = reader->forward_ptr(start);
  const auto end = reader->back_ptr();

  while (forward < end && (*reader)[forward] != '\'') {
    if ((*reader)[forward] == '\\') {
      forward = reader->forward_ptr(forward);
    }
    forward = reader->forward_ptr(forward);
  }

  if (forward > end) {
    report_code_error((*reader)[static_cast<size_t>(forward.l)].code,
        static_cast<size_t>(forward.l),
        static_cast<size_t>(forward.c),
        "Incomplete character literal");
  } else {
    std::string value = (*reader)(reader->forward_ptr(start), forward); // not including the single quotes
    if (value[0] == '\\') {
      if (value.length() != 2) {
        report_code_error((*reader)[static_cast<size_t>(forward.l)].code,
            static_cast<size_t>(forward.l),
            static_cast<size_t>(forward.c),
            "Invalid character literal");
      }
      value = std::string(1, escape_char(value[1]));
    } else if (value.length() != 1) {
      report_code_error((*reader)[static_cast<size_t>(forward.l)].code,
          static_cast<size_t>(forward.l),
          static_cast<size_t>(forward.c),
          "Invalid character literal");
    }
    t = new Token(TokenType::CHAR, value, start, &(*reader)[static_cast<size_t>(start.l)]);
    start = (*reader).forward_ptr(forward);
  }
  return t;
}

// TODO: check line breaks inside two quotation marks
Token *tokenize_string(Reader *reader, code_ptr &start) {
  Token *t = nullptr;
  auto forward = reader->forward_ptr(start);
  const auto end = reader->back_ptr();

  while (forward < end && (*reader)[forward] != '"') {
    if ((*reader)[forward] == '\\') { // escape
      forward = reader->forward_ptr(forward);
    }
    forward = reader->forward_ptr(forward);
  }

  if (forward > end) {
    report_code_error((*reader)[static_cast<size_t>(forward.l)].code,
        static_cast<size_t>(forward.l),
        static_cast<size_t>(forward.c),
        "Incomplete string literal");
  } else {
    std::string value = (*reader)(reader->forward_ptr(start), forward); // not including the double quotes
    std::string escaped = "";
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
    t = new Token(TokenType::STRING, escaped, start, &(*reader)[static_cast<size_t>(start.l)]);
    start = (*reader).forward_ptr(forward);
  }
  return t;
}

Token *tokenize_punctuation(Reader *reader, code_ptr &start) {
  Token *t = nullptr;
  auto next = reader->forward_ptr(start);
  size_t lineno = static_cast<size_t>(start.l);

  if ((*reader)[start] == '/' && ((*reader)[next] == '/' || (*reader)[next] == '*')) { /// line comment or block comment
    t = tokenize_comments(reader, start);
  } else if ((*reader)[start] == '\'') { /// char literal
    t = tokenize_char(reader, start);
  } else if ((*reader)[start] == '"') { /// string literal
    t = tokenize_string(reader, start);
  } else if (std::find(OP.begin(), OP.end(), (*reader)[start]) != OP.end()) { /// operators
    std::string value;
    {
      code_ptr nnext = reader->forward_ptr(next);
      code_ptr nnnext = reader->forward_ptr(nnext);
      code_ptr back_ptr = reader->back_ptr();
      std::string two = (*reader)(start, nnext);
      std::string three = (*reader)(start, reader->forward_ptr(nnext));

      if (next < back_ptr && nnext < back_ptr
          && std::find(OP_ALL.begin(), OP_ALL.end(), three) != OP_ALL.end()) { /// operator containing three characters
        value = (*reader)(start, nnnext);
        start = nnnext;
      } else if (next < back_ptr && std::find(OP_ALL.begin(), OP_ALL.end(), two) != OP_ALL.end()) {
        value = (*reader)(start, nnext);
        if (OPERATION_VALUE_TYPE_MAP.find(value) != OPERATION_VALUE_TYPE_MAP.end()) { /// operator containing two chars
          start = nnext;
        }
      } else {
        /// operator containing one chars
        value = std::string{(*reader)[start]};
        assert(OPERATION_VALUE_TYPE_MAP.find(value) != OPERATION_VALUE_TYPE_MAP.end());
        start = next;
      }
    }
    // create new token, fill in token
    TokenType type = OPERATION_VALUE_TYPE_MAP[value];
    t = new Token(type, value, start, &(*reader)[lineno]);
  } /// other punctuations
  else if (std::find(PUNCTUATIONS.begin(), PUNCTUATIONS.end(), (*reader)[start]) != PUNCTUATIONS.end()) {
    t = new Token(TokenType::PUNCTUATION, std::string(1, (*reader)[start]), start, &(*reader)[lineno]);
    start = next;
  } else {
    t = nullptr;
  }
  return t;
}

std::vector<Token *> tokenize(Reader *reader) {
  code_ptr start = code_ptr(0, 0);
  if (reader->get_n_lines() == 0) { return {}; }
  std::vector<Token *> tokens;
  const auto end = reader->back_ptr();
  while (start < end) {
    /// if start with a letter
    if (std::isalpha((*reader)[start])) {
      auto *new_token = tokenize_keyword(reader, start);
      if (!new_token) {
        /// if this is not a keyword, probably an identifier
        new_token = tokenize_id(reader, start);
        if (!new_token) {
          report_code_error((*reader)[static_cast<size_t>(start.l)].code,
              static_cast<size_t>(start.l),
              static_cast<size_t>(start.c),
              "Invalid identifier");
        }
      }
      tokens.emplace_back(new_token);
    } else if ((*reader)[start] == '_') {
      /// start with an underscore, must be an identifier
      auto *new_token = tokenize_id(reader, start);
      if (!new_token) {
        report_code_error((*reader)[static_cast<size_t>(start.l)].code,
            static_cast<size_t>(start.l),
            static_cast<size_t>(start.c),
            "Invalid identifier");
      }
      tokens.emplace_back(new_token);
    } else if (std::isdigit((*reader)[start])) {
      /// number literal
      auto *new_token = tokenize_number(reader, start);
      if (!new_token) {
        report_code_error((*reader)[static_cast<size_t>(start.l)].code,
            static_cast<size_t>(start.l),
            static_cast<size_t>(start.c),
            "Invalid number literal");
      }
      tokens.emplace_back(new_token);
    } else if (std::find(PUNCTUATIONS.begin(), PUNCTUATIONS.end(), (*reader)[start]) != PUNCTUATIONS.end()) {
      /// punctuations
      auto *new_token = tokenize_punctuation(reader, start);
      if (!new_token) {
        report_code_error((*reader)[static_cast<size_t>(start.l)].code,
            static_cast<size_t>(start.l),
            static_cast<size_t>(start.c),
            "Invalid symbol(s)");
      }
      tokens.emplace_back(new_token);
    } else {
      // start = reader->forward_ptr(start);
      report_code_error((*reader)[static_cast<size_t>(start.l)].code,
          static_cast<size_t>(start.l),
          static_cast<size_t>(start.c),
          "Invalid symbol");
    }
    start = skip_whitespace(reader, start);
  }
  return tokens;
}

} // namespace tanlang
