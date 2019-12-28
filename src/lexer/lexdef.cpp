#include "lexdef.h"

namespace tanlang {

std::unordered_map<TokenType, std::string> token_type_names{
    {TokenType::COMMENTS, "COMMENTS"},
    {TokenType::KEYWORD, "KEYWORD"},
    {TokenType::INT, "INT"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::ID, "ID"},
    {TokenType::CHAR, "CHAR"},
    {TokenType::STRING, "STRING"},
    {TokenType::PUNCTUATION, "PUNCTUATION"},
    {TokenType::RELOP, "RELOP"},
    {TokenType::UOP, "UOP"},
    {TokenType::BOP, "BOP"},
};

const std::vector<std::string> KEYWORDS{
    "for", "while", "do", "if", "else", "fn", "var", "int", "float",
    "continue", "break", "let", "struct", "enum", "union", "switch",
    "case", "str", "u32",
};

const std::vector<char> PUNCTUATIONS{
    '~', '!', '#', '%', '^', '&', '*', '(', ')', '-', '=', '+', '[', ']',
    '{', '}', '\\', '|', ';', ':', '\'', '"', ',', '.', '<', '>', '/', '?',
};

// any symbol in OP can both be an operator itself or the start of an
// operator
const std::vector<char> OP{'~', '!', '%', '^', '&', '*', '-', '=', '+', '|', '<', '>', '/'};

const std::vector<std::string> OP_ALL{
    "==", "!=", ">=", "<=", ">", "<", "&&", "||", "~", "%=", "%", "^=", "^", "&=", "&", "+=", "+", "-=", "-",
    "*=", "*", "/=", "/", "|=", "|", "<<=", "<<", ">>=", ">>", "!=", "."
};

std::unordered_map<std::string, TokenType> OPERATION_VALUE_TYPE_MAP{
    // RELOP
    std::pair("==", TokenType::RELOP), std::pair("!=", TokenType::RELOP), std::pair(">=", TokenType::RELOP),
    std::pair("<=", TokenType::RELOP), std::pair(">", TokenType::RELOP), std::pair("<", TokenType::RELOP),
    std::pair("&&", TokenType::RELOP), std::pair("||", TokenType::RELOP),
    // UOP
    std::pair("~", TokenType::UOP),
    // BOP
    std::pair("%=", TokenType::BOP), std::pair("%", TokenType::BOP), std::pair("^=", TokenType::BOP),
    std::pair("^", TokenType::BOP), std::pair("&=", TokenType::BOP), std::pair("&", TokenType::BOP),
    std::pair("+=", TokenType::BOP), std::pair("+", TokenType::BOP), std::pair("-=", TokenType::BOP),
    std::pair("-", TokenType::BOP), std::pair("*=", TokenType::BOP), std::pair("*", TokenType::BOP),
    std::pair("/=", TokenType::BOP), std::pair("/", TokenType::BOP), std::pair("|=", TokenType::BOP),
    std::pair("|", TokenType::BOP), std::pair("<<=", TokenType::BOP), std::pair("<<", TokenType::BOP),
    std::pair(">>=", TokenType::BOP), std::pair(">>", TokenType::BOP), std::pair("!=", TokenType::BOP),
    std::pair(",", TokenType::BOP), std::pair(".", TokenType::BOP),
    std::pair("=", TokenType::BOP)
};

}
