#include "lexer.h"
#include "reader.h"
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <stdexcept>

namespace tanlang {
    enum TOKEN_TYPE : uint64_t {
        UNKOWN = 0,
        KEYWORD = 1ull,
        ID = 1ull << 2,
        INT = 1ull << 3,
        FLOAT = 1ull << 4,
        STR_LITERAL = 1ull << 5,
        CHAR = 1ull << 6,
        // unary operatoullRs
        POSITIVE = 1ull << 7,
        NEGATIVE = 1ull << 8,
        LOG_NOT = 1ull << 9,
        BIT_NOT = 1ull << 10,
        ADR_REF = 1ull << 11,
        ADR_DEREF = 1ull << 12,
        // binary operatullOrs
        LSHIFT = 1ull << 14,
        RSHIFT = 1ull << 15,
        EQ = 1ull << 16,
        NE = 1ull << 17,
        LT = 1ull << 18,
        LE = 1ull << 19,
        GT = 1ull << 20,
        GE = 1ull << 21,
        LAND = 1ull << 22,
        BAND = 1ull << 23,
        LOR = 1ull << 24,
        BOR = 1ull << 25,
        BXOR = 1ull << 26,
        ADD = 1ull << 27,
        SUB = 1ull << 28,
        MUL = 1ull << 29,
        DIV = 1ull << 30,
        MOD = 1ull << 31,
        // assignment operators
        LSHIFT_ASSIGN = 1ull << 32,
        RSHIFT_ASSIGN = 1ull << 33,
        BAND_ASSIGN = 1ull << 34,
        BOR_ASSIGN = 1ull << 35,
        BXOR_ASSIGN = 1ull << 36,
        ADD_ASSIGN = 1ull << 37,
        SUB_ASSIGN = 1ull << 38,
        MUL_ASSIGN = 1ull << 39,
        DIV_ASSIGN = 1ull << 40,
        MOD_ASSIGN = 1ull << 41,
    };

    struct token_info {
        TOKEN_TYPE type;
        union {
            std::string str;
            uint64_t val = 0;
        };
        token_info() {}
        ~token_info(){};
    };

    Lexer::Lexer() { _reader = std::unique_ptr<Reader>(new Reader); }

    Lexer::~Lexer() {
        for (auto *&t : _token_infos) {
            if (t != nullptr) {
                delete t;
                t = nullptr;
            }
        }
    }

    token_info *advance_for_number(const std::string &str, size_t &current,
                                   size_t max_len) {
        assert(std::isdigit(str[current]));
        const size_t start = current;
        size_t curr = current;
        uint64_t val = 0;
        if (str[curr] == '0') {       // hex or binary or 0
            if (curr + 1 < max_len) { // hex or binary
                if (str[curr + 1] == 'x' || str[curr + 1] == 'X') { // hex
                    curr += 2;
                    char c = str[curr];
                    // @Consider maybe std::stoull is faster? Need benchmarking.
                    while (curr < max_len && std::isxdigit(c)) {
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
                    while (curr < max_len && (c == '1' || c == '0')) {
                        if (c == '1') {
                            val = val * 2 + 1;
                        } else {
                            val *= 2;
                        }
                        c = str[++curr];
                    }
                    if (curr - start == 2) // only "0b", then still dec
                        goto dec;
                }
            } else { // 0
                val = 0;
                goto ret;
            }
        } else {
        dec:
            while (std::isdigit(str[curr++])) {}
            // borrow C++11 std
            std::string subs = str.substr(start, curr - start);
            val = std::stoull(subs);
        }
    ret:
        auto *t = new token_info;
        t->type = INT;
        t->val = val;
        assert(start ==
               current); // we must not change current's value until now
        current = current - 1;
        return t;
    }

    void Lexer::open(const std::string &file_name) { _reader->open(file_name); }

    void Lexer::lex() {
        std::string line = _reader->next_line();
        while (!line.empty()) {
            size_t start = 0, current = 0;
            const size_t line_len = line.length();
            while (true) {
                if (current >= line_len) {
                    break;
                } else if (std::isdigit(line[current])) {
                    auto *t = advance_for_number(line, current, line_len);
                    _token_infos.emplace_back(t);
                    if (current >= line_len)
                        break; // EOF
                } else if (std::isalpha(line[current]) ||
                           line[current] == '_') {
                } else {
                    // TODO: recognize identifiers and keywords
                }
                ++current;
            }
            line = _reader->next_line();
        }
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
                os << "; Value: " << t->val;
            } else {
                os << "; Value: " << t->str;
            }
        }
        return os;
    }
#endif

} // namespace tanlang
