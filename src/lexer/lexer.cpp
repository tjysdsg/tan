#include "lexer.h"
#include "src/lexer/lexer_internal.h"
#include <cassert>
#include <limits>
#include <iomanip>
#include "base.h"

namespace tanlang {

    Lexer::Lexer() : _reader(std::unique_ptr<Reader>(new Reader)) {}

    Lexer::~Lexer() {
        for (auto *&t : _token_infos) {
            if (t != nullptr) {
                delete t;
                t = nullptr;
            }
        }
    }

    token_info *advance_for_number(const std::string &str, size_t &current,
                                   size_t len) {
        assert(std::isdigit(str[current]));
        const size_t start = current;
        size_t curr = current;
        union {
            uint64_t val = 0;
            double fval;
        };
        bool is_float = false;
        if (str[curr] == '0') {   // hex or binary or 0
            if (curr + 1 < len) { // hex or binary
                if (str[curr + 1] == 'x' || str[curr + 1] == 'X') { // hex
                    curr += 2;
                    char c = str[curr];
                    // @Consider maybe std::stoull is faster? Need benchmarking.
                    while (curr < len && std::isxdigit(c)) {
                        if (c <= 'F' && c >= 'A') { // ABCDEF
                            val = 16u * val + (uint64_t)(c - 'A' + 10);
                        } else if (c <= 'f' && c >= 'a') { // abcdef
                            val = 16u * val + (uint64_t)(c - 'a' + 10);
                        } else { // 0123456789
                            val = 16u * val + (uint64_t)(c - '0');
                        }
                        c = str[++curr];
                    }
                    if (curr - start == 2) // only "0x", then still dec
                        goto dec;
                } else if (str[curr + 1] == 'b' ||
                           str[curr + 1] == 'B') { // binary
                    curr += 2;
                    char c = str[curr];
                    while (curr < len && (c == '1' || c == '0')) {
                        if (c == '1') {
                            val = val * 2 + 1;
                        } else {
                            val *= 2;
                        }
                        c = str[++curr];
                    }
                    if (curr - start == 2) // only "0b", then still dec
                        goto dec;
                } else
                    // necessary, otherwise in `ret:` current = curr - 1 will
                    // cause infinite loop
                    ++curr;
            } else { // 0
                val = 0;
                ++curr;
                goto ret;
            }
        } else {
        dec:
            while (curr < len) {
                char ch = str[curr];
                if (ch == '.') {
                    is_float = true;
                } else if (!(std::isdigit(ch) || ch == 'e' ||
                             ch == 'E')) { // if it's not float number or
                                           // decimal integer
                    break;
                }
                curr++;
            }
            std::string subs = str.substr(start, curr - start);
            if (is_float) {
                // FIXME: might want to use custom version, depending on whether
                // I can boost the performance
                fval = std::stod(subs);
            } else {
                val = std::stoull(subs);
            }
        }
    ret:
        auto *t = new token_info;
        t->type = is_float ? FLOAT : INT;
        if (is_float)
            t->fval = fval;
        else
            t->val = val;
        current = curr - 1;
        return t;
    }

    token_info *advance_for_identifier(const std::string &str, size_t &current,
                                       size_t len) {
        assert(std::isalpha(str[current]) || str[current] == '_');
        const size_t start = current;
        size_t curr = current + 1;
        while (true) {
            if (curr >= len)
                break;
            char ch = str[curr];
            if (std::isalpha(ch) || std::isdigit(ch) || ch == '_')
                ++curr;
            else
                break;
        }
        auto *new_token = new token_info;
        new_token->type = ID;
        new_token->str = str.substr(start, curr - start);
        current = curr - 1;
        return new_token;
    }

#define _X_OR_XY_(x, y, type_x, type_y)                                        \
    case x:                                                                    \
        if (curr + 1 < len && str[curr + 1] == (y)) {                          \
            type = (TOKEN_TYPE)((type_x) | (type_y));                          \
            ++curr;                                                            \
        } else {                                                               \
            type = (TOKEN_TYPE)(type_x);                                       \
        }                                                                      \
        goto ret;                                                              \
        break;

#define _X_OR_DOUBLE_X_(x, type_x)                                             \
    case x:                                                                    \
        if (curr + 1 < len && str[curr + 1] == (x)) {                          \
            type = (TOKEN_TYPE)((type_x) + 1);                                 \
            ++curr;                                                            \
        } else {                                                               \
            type = (TOKEN_TYPE)(type_x);                                       \
        }                                                                      \
        goto ret;                                                              \
        break;

#define _X_OR_DOUBLE_X_OR_DOUBLE_X_Y_OR_X_Y_(x, type_x, y, type_y)             \
    case x:                                                                    \
        if (curr + 1 < len && str[curr + 1] == (x)) {                          \
            type = (TOKEN_TYPE)((type_x) + 1);                                 \
            ++curr;                                                            \
            if (curr + 1 < len && str[curr + 1] == (y)) {                      \
                type = (TOKEN_TYPE)(((type_x) + 1) | (type_y));                \
                ++curr;                                                        \
            }                                                                  \
        } else if (curr + 1 < len && str[curr + 1] == (y)) {                   \
            type = (TOKEN_TYPE)((type_x) | (type_y));                          \
            ++curr;                                                            \
        } else {                                                               \
            type = (TOKEN_TYPE)(type_x);                                       \
        }                                                                      \
        goto ret;                                                              \
        break;

#define _X_OR_DOUBLE_X_OR_X_Y_(x, type_x, y, type_y)                           \
    case x:                                                                    \
        if (curr + 1 < len && str[curr + 1] == (x)) {                          \
            type = (TOKEN_TYPE)((type_x) + 1);                                 \
            ++curr;                                                            \
        } else if (curr + 1 < len && str[curr + 1] == (y)) {                   \
            type = (TOKEN_TYPE)((type_x) | (type_y));                          \
            ++curr;                                                            \
        } else {                                                               \
            type = (TOKEN_TYPE)(type_x);                                       \
        }                                                                      \
        goto ret;                                                              \
        break;

    token_info *advance_for_symbol(const std::string &str, size_t &current,
                                   size_t len) {
        size_t curr = current;
        TOKEN_TYPE type = UNKOWN;
        while (true) {
            if (curr >= len)
                break;
            char ch = str[curr];
            switch (ch) {
                _X_OR_XY_('!', '=', EXCLAIM, EQ)
                _X_OR_DOUBLE_X_OR_X_Y_('|', BAR, '=', EQ)
                _X_OR_DOUBLE_X_OR_X_Y_('&', AND, '=', EQ)
                _X_OR_XY_('^', '=', CARET, EQ)
                _X_OR_XY_('+', '=', PLUS, EQ)
                _X_OR_XY_('-', '=', MINUS, EQ)
                _X_OR_XY_('*', '=', STAR, EQ)
                _X_OR_XY_('~', '=', TILDE, EQ)
                _X_OR_XY_('/', '=', SLASH, EQ)
                _X_OR_XY_('%', '=', PERCENT, EQ)
                _X_OR_DOUBLE_X_('=', EQ)
                _X_OR_DOUBLE_X_OR_DOUBLE_X_Y_OR_X_Y_('>', GT, '=', EQ)
                _X_OR_DOUBLE_X_OR_DOUBLE_X_Y_OR_X_Y_('<', LT, '=', EQ)
            default:
                goto ret;
                break;
            }
            ++curr;
        }
    ret:
        auto *new_token = new token_info;
        new_token->type = type;
        current = curr;
        return new_token;
    }

    token_info *advance_for_string_literal(const std::string &str,
                                           size_t &current, size_t len) {
        assert(str[current] == '"');
        ++current; // omit the first "
        const size_t start = current;
        size_t curr = current + 1;
        while (true) {
            if (curr >= len)
                break;
            char ch = str[curr];
            if (ch == '"')
                break;
            else
                ++curr;
        }
        auto *new_token = new token_info;
        new_token->type = STR_LITERAL;
        new_token->str = str.substr(start, curr - start);
        current = curr; // omit the second "
        return new_token;
    }

    void Lexer::open(const std::string &file_name) { _reader->open(file_name); }

    void Lexer::lex() {
        std::string line = _reader->next_line();
        do {
            size_t /*start = 0,*/ current = 0;
            const size_t line_len = line.length();
            while (true) {
                if (current >= line_len) {
                    break;
                } /* else if (is_whitespace(line[current])) {
                 } */
                // Reader can make sure that the first character must not be
                // whitespace, so we don't have to check here.
                else if (std::isdigit(line[current])) { // number literal
                    auto *t = advance_for_number(line, current, line_len);
                    _token_infos.emplace_back(t);
                } else if (std::isalpha(line[current]) ||
                           line[current] == '_') { // identifier
                    auto *t = advance_for_identifier(line, current, line_len);
                    // TODO: lex keywords
                    _token_infos.emplace_back(t);
                } else if (line[current] == '\'') { // char
                    ++current;
                    auto *t = new token_info;
                    t->type = CHAR;
                    t->val = (uint64_t)line[current];
                    ++current;
                    if (line[current] != '\'') {
                        // TODO error handling
                    }
                    _token_infos.emplace_back(t);
                } else if (line[current] == '"') { // string literals
                    auto *t =
                        advance_for_string_literal(line, current, line_len);
                    _token_infos.emplace_back(t);
                } else { // symbols
                    auto *t = advance_for_symbol(line, current, line_len);
                    _token_infos.emplace_back(t);
                }
                ++current;
            }
            line = _reader->next_line();
        } while (!_reader->eof());
    }

    token_info *Lexer::next_token() const {
        if (++_curr_token > _token_infos.size())
            return nullptr;
        else
            return _token_infos[_curr_token - 1];
    }

    token_info *Lexer::get_token(const unsigned idx) const {
        _curr_token = idx;
        if (_curr_token - 1 > _token_infos.size())
            return nullptr;
        else
            return _token_infos[_curr_token - 1];
    }

#ifdef DEBUG_ENABLED
    void Lexer::read_string(const std::string &code) {
        _reader->read_string(code);
    }
    std::ostream &operator<<(std::ostream &os, const Lexer &lexer) {
        for (auto *t : lexer._token_infos) {
            os << "TOKEN_TYPE: " << t->type;
            if (t->type == INT) {
                os << "; Value: " << t->val << "\n";
            } else if (t->type == FLOAT) {
                os << "; Value: " << t->fval << "\n";
            } else if (t->type == CHAR) {
                os << "; Value: " << char(t->val) << "\n";
            } else {
                os << "; Value: " << t->str << "\n";
            }
        }
        return os;
    }
#endif

} // namespace tanlang
