#include "source_file/token.h"

namespace tanlang {

/// map TokenType to string
umap<TokenType, str> token_type_names{
    {TokenType::COMMENTS,    "COMMENTS"   },
    {TokenType::KEYWORD,     "KEYWORD"    },
    {TokenType::INT,         "INT"        },
    {TokenType::FLOAT,       "FLOAT"      },
    {TokenType::ID,          "ID"         },
    {TokenType::CHAR,        "CHAR"       },
    {TokenType::STRING,      "STRING"     },
    {TokenType::PUNCTUATION, "PUNCTUATION"},
    {TokenType::RELOP,       "RELOP"      },
    {TokenType::UOP,         "UOP"        },
    {TokenType::BOP,         "BOP"        },
};

/// keywords/reserved words
const vector<str> KEYWORDS{
    "for",    "while", "if",     "else", "fn",     "var",    "continue", "break", "let",   "struct",  "union",
    "switch", "case",  "return", "pub",  "extern", "import", "as",       "true",  "false", "package",
};

const vector<char> PUNCTUATIONS{'~', '!',  '#', '%', '^', '&',  '*', '(', ')', '-', '=', '+', '[', ']', '{',
                                '}', '\\', '|', ';', ':', '\'', '"', ',', '.', '<', '>', '/', '?', '@'};

const vector<str> TERMINAL_TOKENS{";", "}", ")", ":", ",", "]"};

/// any symbol in OP can both be an operator itself or the first character of an operator
const vector<char> OP{'~', '!', '%', '^', '&', '*', '-', '=', '+', '|', '<', '>', '/', '.'};

const vector<str> OP_ALL{"==", "!=", ">=", "<=", ">", "<",  "&&", "||", "~", "%=",  "%",  "^=",  "^",  "&=", "&", "+=",
                         "+",  "-=", "-",  "*=", "*", "/=", "/",  "|=", "|", "<<=", "<<", ">>=", ">>", "!=", "."};

umap<str, TokenType> OPERATION_VALUE_TYPE_MAP{
    // RELOP
    pair("==", TokenType::RELOP), pair("!=", TokenType::RELOP), pair(">=", TokenType::RELOP),
    pair("<=", TokenType::RELOP), pair(">", TokenType::RELOP), pair("<", TokenType::RELOP),
    pair("&&", TokenType::RELOP), pair("||", TokenType::RELOP),
    // UOP
    pair("~", TokenType::UOP), pair("!", TokenType::UOP),
    // BOP
    pair("%=", TokenType::BOP), pair("%", TokenType::BOP), pair("^=", TokenType::BOP), pair("^", TokenType::BOP),
    pair("&=", TokenType::BOP), pair("&", TokenType::BOP), pair("+=", TokenType::BOP), pair("+", TokenType::BOP),
    pair("-=", TokenType::BOP), pair("-", TokenType::BOP), pair("*=", TokenType::BOP), pair("*", TokenType::BOP),
    pair("/=", TokenType::BOP), pair("/", TokenType::BOP), pair("|=", TokenType::BOP), pair("|", TokenType::BOP),
    pair("<<=", TokenType::BOP), pair("<<", TokenType::BOP), pair(">>=", TokenType::BOP), pair(">>", TokenType::BOP),
    pair("!=", TokenType::BOP), pair(",", TokenType::BOP), pair(".", TokenType::BOP), pair("=", TokenType::BOP)};

Token::Token(TokenType tokenType, uint32_t line, uint32_t col, str value, SourceFile *src)
    : _type(tokenType), _value(std::move(value)), _line(line), _col(col), _src(src) {}

TokenType Token::get_type() const { return _type; }

void Token::set_type(TokenType type) { _type = type; }

const str &Token::get_value() const { return _value; }

str Token::get_source_line() const { return _src->get_line(_line); }

bool Token::is_unsigned() const { return _is_unsigned; }

void Token::set_is_unsigned(bool is_unsigned) { _is_unsigned = is_unsigned; }

uint32_t Token::get_line() const { return _line; }

uint32_t Token::get_col() const { return _col; }

SourceSpan Token::GetSourceSpan(const Token &start, const Token &end) {
  Cursor c1(start._line, start._col, start._src);
  Cursor c2(end._line, end._col, end._src);
  return SourceSpan(c1, c2);
}

} // namespace tanlang
