#ifndef TAN_READER_READER_H
#define TAN_READER_READER_H
#include "config.h"
#include "utils.h"
#include <cstdint>
#include <string>
#include <unistd.h>
#include <vector>
namespace tanlang {
    struct code_ptr {
        long r = 0;
        long c = 0;
        code_ptr() = default;
        code_ptr(long r, long c) : r(r), c(c) {}
        code_ptr(const code_ptr &other) {
            r = other.r;
            c = other.c;
        }
        ~code_ptr() = default;
        bool operator==(const code_ptr &other) { return r == other.r && c == other.c; }
        bool operator<(const code_ptr &other) {
            if (r < other.r) {
                return true;
            } else if (r > other.r) {
                return false;
            } else {
                return c < other.c;
            }
        }
        bool operator>(const code_ptr &other) {
            if (r > other.r) {
                return true;
            } else if (r < other.r) {
                return false;
            } else {
                return c > other.c;
            }
        }
        code_ptr &operator=(const code_ptr &other) {
            r = other.r;
            c = other.c;
            return *this;
        }
        static code_ptr npos() { return code_ptr(-1, -1); }
        bool operator!=(const code_ptr &other) { return !(*this == other); }
    };

    struct line_info {
        unsigned lineno = 0;   // line number
        std::string code = ""; // actual code
        line_info() = default;
        explicit line_info(const unsigned lineno) { this->lineno = lineno; }
        ~line_info() = default;
    };

    class Reader;

    // TODO: support unicode
    class Reader final {
      public:
        Reader() = default;
        ~Reader();
        void open(const std::string &filename);
        void from_string(const std::string &code);
        // bool set_encoding(const std::string& encoding);
        [[nodiscard]] std::string get_filename() const;
        [[nodiscard]] size_t size() const { return _lines.size(); }
        line_info &operator[](const size_t index) const {
            assert(index < _lines.size());
            return *(_lines[index]);
        }
        char operator[](const code_ptr &ptr) const {
            assert(ptr.r >= 0 && ptr.c >= 0);
            return _lines[static_cast<size_t>(ptr.r)]->code[static_cast<size_t>(ptr.c)];
        }
        // start inclusive, end exclusive string slice
        std::string operator()(const code_ptr &start, code_ptr end = code_ptr::npos()) const {
            assert(start.r >= 0 && start.c >= 0);
            assert(end.r >= -1 && end.c >= -1);
            // if end can contain -1 only if r and c are both -1
            assert(!((end.r == -1) ^ (end.c == -1)));
            if (end.r == -1 && end.c == -1) {
                end.r = static_cast<long>(_lines.size() - 1);
                end.c = static_cast<long>(_lines[static_cast<size_t>(end.r)]->code.length());
            }
            auto s_row = start.r;
            auto e_row = end.r;
            std::string ret;
            if (s_row == e_row) {
                assert(start.c != end.c);
                ret = _lines[static_cast<size_t>(s_row)]->code.substr(static_cast<unsigned long>(start.c),
                                                                      static_cast<unsigned long>(end.c - start.c));
            } else {
                ret += _lines[static_cast<size_t>(s_row)]->code.substr(static_cast<unsigned long>(start.c));
                for (auto r = s_row; r < e_row - 1; ++r) {
                    ret += _lines[static_cast<size_t>(r)]->code + "\n";
                }
                ret += _lines[static_cast<size_t>(e_row)]->code.substr(0, static_cast<size_t>(end.c));
            }
            return ret;
        }
        [[nodiscard]] code_ptr front_ptr() const { return code_ptr(0, 0); }
        [[nodiscard]] code_ptr back_ptr() const {
            return code_ptr(static_cast<long>(_lines.size() - 1), static_cast<long>(_lines.back()->code.length() - 1));
        }
        // return a copy of code_ptr that points to the next position of ptr
        [[nodiscard]] code_ptr forward_ptr(code_ptr ptr) {
            long n_cols = static_cast<long>(_lines[static_cast<size_t>(ptr.r)]->code.length());
            if (ptr.c == n_cols - 1) {
                if (ptr.r < static_cast<long>(_lines.size())) {
                    ++ptr.r;
                }
                ptr.c = 0;
            } else {
                ++ptr.c;
            }
            return ptr;
        }

      private:
        std::string _filename = "";
        std::vector<line_info *> _lines{};
    };
} // namespace tanlang

#endif // TAN_READER_READER_H
