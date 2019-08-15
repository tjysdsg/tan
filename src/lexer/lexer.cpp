#include "lexer.h"
#include "base.h"
#include "lexdef.h"
#include <algorithm>
#include <cctype>
#include <regex>

namespace tanlang {
    code_ptr skip_whitespace(Reader *reader, code_ptr ptr) {
        const auto end = reader->back_ptr();
        while (ptr != end && std::isspace((*reader)[ptr])) {
            ptr = reader->forward_ptr(ptr);
        }
        return ptr;
    }
    code_ptr skip_until(Reader *reader, code_ptr ptr, const char delim) {
        const auto end = reader->back_ptr();
        while (ptr != end && (*reader)[ptr] != delim) {
            ptr = reader->forward_ptr(ptr);
        }
        return ptr;
    }
    /*
     * NOTE: INVARIANT for all tokenize_xx functions
     *      start is at least one token before the end
     * NOTE: call of tokenize_keyword must before that of
     *      tokenize_id
     * */

    token *tokenize_id(Reader *reader, code_ptr &start) {
        token *ret = nullptr;
        auto forward = start;
        const auto end = reader->back_ptr();
        while (forward != end) {
            if (std::isalnum((*reader)[forward]) || (*reader)[forward] == '_') {
                forward = reader->forward_ptr(forward);
            } else {
                ret = new token(TokenType::ID, (*reader)(start, forward));
                break;
            }
        }
        start = forward;
        return ret;
    }
    token *tokenize_keyword(Reader *reader, code_ptr &start) {
        // find whether the value is in KEYWORDS (lexdef.h) based on
        // returned value of tokenize_id()
        auto *t = tokenize_id(reader, start);
        if (t) {
            if (std::find(std::begin(KEYWORDS), std::end(KEYWORDS), t->value) != std::end(KEYWORDS)) {
                t->type = TokenType::KEYWORD;
            } else {
                delete (t);
                t = nullptr;
            }
        }
        return t;
    }
    token *tokenize_comments(Reader *reader, code_ptr &start) {
        token *t = nullptr;
        auto next = reader->forward_ptr(start);
        if ((*reader)[next] == '/') {
            // line comments
            auto value = (*reader)(reader->forward_ptr(next));
            t = new token(TokenType::COMMENTS, value);
            start.c = static_cast<long>((*reader)[static_cast<size_t>(start.r)].code.length());
            start = (*reader).forward_ptr(start);
        } else if ((*reader)[next] == '*') {
            /* block commments */
            auto forward = start;
            // loop for each line
            while (static_cast<size_t>(forward.r) < reader->size()) {
                auto re = std::regex(R"(.*\*\/)");
                auto str = (*reader)(forward);
                std::smatch result;
                if (std::regex_match(str, result, re)) {
                    std::string value = str.substr(2, static_cast<size_t>(result.length(0) - 4));
                    t = new token(TokenType::COMMENTS, value);
                    forward.c = result.length(0);
                    start = forward;
                }
                ++forward.r;
            }
            if (!t) {
                report_code_error((*reader)[static_cast<size_t>(start.r)].code, static_cast<size_t>(start.r),
                                  static_cast<size_t>(start.c), "Invalid comments");
            }
        } else {
            report_code_error((*reader)[static_cast<size_t>(start.r)].code, static_cast<size_t>(start.r),
                              static_cast<size_t>(start.c), "Invalid comments");
        }
        return t;
    }
    // TODO: implement tokenize_int, tokenize_float, tokenize_number
    token *tokenize_int(Reader *reader, code_ptr &start) {
        auto forward = start;
        const auto end = reader->back_ptr();
        return nullptr;
    }
    token *tokenize_float(Reader *reader, code_ptr &start) {
        auto forward = start;
        const auto end = reader->back_ptr();
        return nullptr;
    }
    token *tokenize_number(Reader *reader, code_ptr &start) {
        auto forward = start;
        const auto end = reader->back_ptr();
        return nullptr;
    }
    // TODO: support escape sequences inside char literals
    token *tokenize_char(Reader *reader, code_ptr &start) {
        token *t = nullptr;
        auto forward = start;
        const auto end = reader->back_ptr();
        forward = skip_until(reader, forward, '\'');
        if (forward == end) {
            report_code_error((*reader)[static_cast<size_t>(forward.r)].code, static_cast<size_t>(forward.r),
                              static_cast<size_t>(forward.c), "Incomplete character literal");
            exit(1);
        } else {
            std::string value = (*reader)(reader->forward_ptr(start),
                                          forward); // not including the single quotes
            t = new token(TokenType::CHAR, value);
            start = (*reader).forward_ptr(forward);
        }
        return t;
    }
    // TODO: support escape sequences inside string literals
    token *tokenize_string(Reader *reader, code_ptr &start) {
        token *t = nullptr;
        auto forward = start;
        const auto end = reader->back_ptr();
        forward = skip_until(reader, forward, '"');
        if (forward == end) {
            report_code_error((*reader)[static_cast<size_t>(forward.r)].code, static_cast<size_t>(forward.r),
                              static_cast<size_t>(forward.c), "Incomplete string literal");
            exit(1);
        } else {
            std::string value = (*reader)(reader->forward_ptr(start),
                                          forward); // not including the double quotes
            t = new token(TokenType::STRING, value);
            start = (*reader).forward_ptr(forward);
        }
        return t;
    }
    token *tokenize_punctuation(Reader *reader, code_ptr &start) {
        token *t = nullptr;
        auto next = reader->forward_ptr(start);

        // line comment or block comment
        if ((*reader)[start] == '/' && ((*reader)[next] == '/' || (*reader)[next] == '*')) {
            t = tokenize_comments(reader, start);
        }
        // char literal
        else if ((*reader)[start] == '\'') {
            t = tokenize_char(reader, start);
        }
        // string literal
        else if ((*reader)[start] == '"') {
            t = tokenize_string(reader, start);
        }
        // operators
        else if (std::find(OP_SINGLE.begin(), OP_SINGLE.end(), (*reader)[start]) != OP_SINGLE.end()) {
            std::string value;
            do {
                if (std::find(OP.begin(), OP.end(), (*reader)[next]) == OP.end()) {
                    // operator containing one char
                    value = std::string(1, (*reader)[start]);
                    start = next;
                    break;
                }
                code_ptr nnext = reader->forward_ptr(next);
                if (nnext == reader->back_ptr() || std::find(OP.begin(), OP.end(), (*reader)[nnext]) == OP.end()) {
                    value = std::string{(*reader)[start], (*reader)[next]};
                    if (OPERATION_VALUE_TYPE_MAP.contains(value.c_str())) {
                        // operator containing two chars
                        start = nnext;
                        break;
                    }
                }

                // operator containing three chars
                value = std::string{(*reader)[start], (*reader)[next], (*reader)[nnext]};
                assert(OPERATION_VALUE_TYPE_MAP.contains(value.c_str()));
                start = (*reader).forward_ptr(nnext);
            } while (false);
            // create new token, fill in token
            TokenType type = OPERATION_VALUE_TYPE_MAP[value.c_str()];
            t = new token(type, value);
        }
        // other punctuations
        else if (std::find(PUNCTUATIONS.begin(), PUNCTUATIONS.end(), (*reader)[start]) != PUNCTUATIONS.end()) {
            t = new token(TokenType::PUNCTUATION, std::string(1, (*reader)[start]));
            start = next;
        } else {
            t = nullptr;
        }
        return t;
    }
    std::vector<token *> tokenize(Reader *reader, code_ptr start) {
        // TODO: DO NOT exit the program when errors occurred
        std::vector<token *> tokens;
        const auto end = reader->back_ptr();
        while (start < end) {
            // if start with a letter
            if (std::isalpha((*reader)[start])) {
                auto *new_token = tokenize_keyword(reader, start);
                if (!new_token) {
                    // if this is not a keyword, probably an identifier
                    new_token = tokenize_id(reader, start);
                    if (!new_token) {
                        report_code_error((*reader)[static_cast<size_t>(start.r)].code, static_cast<size_t>(start.r),
                                          static_cast<size_t>(start.c), "Invalid identifier");
                        exit(1);
                    }
                }
                tokens.emplace_back(new_token);
            } else if ((*reader)[start] == '_') {
                // start with an underscore, must be an identifier
                auto *new_token = tokenize_id(reader, start);
                if (!new_token) {
                    report_code_error((*reader)[static_cast<size_t>(start.r)].code, static_cast<size_t>(start.r),
                                      static_cast<size_t>(start.c), "Invalid identifier");
                    exit(1);
                }
                tokens.emplace_back(new_token);
            } else if (std::isdigit((*reader)[start])) {
                // number literal
                auto *new_token = tokenize_number(reader, start);
                if (!new_token) {
                    report_code_error((*reader)[static_cast<size_t>(start.r)].code, static_cast<size_t>(start.r),
                                      static_cast<size_t>(start.c), "Invalid number literal");
                    exit(1);
                }
                tokens.emplace_back(new_token);
            } else if (std::find(std::begin(PUNCTUATIONS), std::end(PUNCTUATIONS), (*reader)[start]) !=
                       std::end(PUNCTUATIONS)) {
                // punctuations
                auto *new_token = tokenize_punctuation(reader, start);
                if (!new_token) {
                    report_code_error((*reader)[static_cast<size_t>(start.r)].code, static_cast<size_t>(start.r),
                                      static_cast<size_t>(start.c), "Invalid symbol(s)");
                    exit(1);
                }
                tokens.emplace_back(new_token);
            }
            start = skip_whitespace(reader, start);
        }
        return tokens;
    }
} // namespace tanlang
