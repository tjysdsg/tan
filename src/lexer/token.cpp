#include "lexer/token.h"

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
const vector<str> KEYWORDS{"for",    "while", "if",     "else",   "fn",    "var",    "continue",
                           "break",  "let",   "struct", "enum",   "union", "switch", "case",
                           "return", "pub",   "extern", "import", "as",    "true",   "false"};

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

str Token::to_string() const {
  return fmt::format("<'{}' {} L{}:C{}>", _value, token_type_names[_type], std::to_string(_loc->get_line()),
                     std::to_string(_loc->get_col()));
}

std::ostream &Token::operator<<(std::ostream &os) const {
  os << to_string();
  return os;
}

Token::~Token() { delete _loc; }

Token::Token(TokenType tokenType, size_t line, size_t col, str value, str source_line)
    : _type(tokenType), _value(std::move(value)), _loc(new SourceLoc(line, col)), _source_line(std::move(source_line)) {
}

TokenType Token::get_type() const { return _type; }

void Token::set_type(TokenType type) { _type = type; }

const str &Token::get_value() const { return _value; }

SourceLoc *Token::get_loc() const { return _loc; }

const str &Token::get_source_line() const { return _source_line; }

bool Token::is_unsigned() const { return _is_unsigned; }

void Token::set_is_unsigned(bool is_unsigned) { _is_unsigned = is_unsigned; }

size_t Token::get_line() const { return _loc->get_line(); }

size_t Token::get_col() const { return _loc->get_col(); }

} // namespace tanlang
