#include "token.h"
#include "reader.h"

namespace tanlang {

/// map TokenType to string
umap<TokenType, str> token_type_names
    {{TokenType::COMMENTS, "COMMENTS"}, {TokenType::KEYWORD, "KEYWORD"}, {TokenType::INT, "INT"},
        {TokenType::FLOAT, "FLOAT"}, {TokenType::ID, "ID"}, {TokenType::CHAR, "CHAR"}, {TokenType::STRING, "STRING"},
        {TokenType::PUNCTUATION, "PUNCTUATION"}, {TokenType::RELOP, "RELOP"}, {TokenType::UOP, "UOP"},
        {TokenType::BOP, "BOP"},};

/// keywords/reserved words
const vector<str> KEYWORDS
    {"for", "while", "if", "else", "fn", "var", "continue", "break", "let", "struct", "enum", "union", "switch", "case",
        "return", "pub", "extern", "import", "as"};

const vector<char> PUNCTUATIONS
    {'~', '!', '#', '%', '^', '&', '*', '(', ')', '-', '=', '+', '[', ']', '{', '}', '\\', '|', ';', ':', '\'', '"',
        ',', '.', '<', '>', '/', '?', '@'};

/// any symbol in OP can both be an operator itself or the first character of an operator
const vector<char> OP{'~', '!', '%', '^', '&', '*', '-', '=', '+', '|', '<', '>', '/', '.'};

const vector<str> OP_ALL
    {"==", "!=", ">=", "<=", ">", "<", "&&", "||", "~", "%=", "%", "^=", "^", "&=", "&", "+=", "+", "-=", "-", "*=",
        "*", "/=", "/", "|=", "|", "<<=", "<<", ">>=", ">>", "!=", "."};

umap<str, TokenType> OPERATION_VALUE_TYPE_MAP{
    // RELOP
    std::pair("==", TokenType::RELOP), std::pair("!=", TokenType::RELOP), std::pair(">=", TokenType::RELOP),
    std::pair("<=", TokenType::RELOP), std::pair(">", TokenType::RELOP), std::pair("<", TokenType::RELOP),
    std::pair("&&", TokenType::RELOP), std::pair("||", TokenType::RELOP),
    // UOP
    std::pair("~", TokenType::UOP), std::pair("!", TokenType::UOP),
    // BOP
    std::pair("%=", TokenType::BOP), std::pair("%", TokenType::BOP), std::pair("^=", TokenType::BOP),
    std::pair("^", TokenType::BOP), std::pair("&=", TokenType::BOP), std::pair("&", TokenType::BOP),
    std::pair("+=", TokenType::BOP), std::pair("+", TokenType::BOP), std::pair("-=", TokenType::BOP),
    std::pair("-", TokenType::BOP), std::pair("*=", TokenType::BOP), std::pair("*", TokenType::BOP),
    std::pair("/=", TokenType::BOP), std::pair("/", TokenType::BOP), std::pair("|=", TokenType::BOP),
    std::pair("|", TokenType::BOP), std::pair("<<=", TokenType::BOP), std::pair("<<", TokenType::BOP),
    std::pair(">>=", TokenType::BOP), std::pair(">>", TokenType::BOP), std::pair("!=", TokenType::BOP),
    std::pair(",", TokenType::BOP), std::pair(".", TokenType::BOP), std::pair("=", TokenType::BOP)};

str Token::to_string() const {
  return "<Token " + token_type_names[type] + " L" + std::to_string(l) + ":C" + std::to_string(c) + ">: " + value;
}

std::ostream &Token::operator<<(std::ostream &os) const {
  os << to_string();
  return os;
}

Token::Token(TokenType tokenType, str value, const cursor &cursor, const line_info *line)
    : type(tokenType), value(std::move(value)), l(cursor.l), c(cursor.c), line(c_cast(line_info*, line)) {}

} // namespace tanlang
