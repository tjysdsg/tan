#ifndef TAN_READER_READER_H
#define TAN_READER_READER_H
#include "config.h"
#include "utils.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace tanlang {
    struct line_info {
        unsigned lineno = 0; // line number
        std::string code;    // actual code
        line_info() = default;
        explicit line_info(const unsigned lineno) { this->lineno = lineno; }
    };

    class Reader;

    // TODO: support unicode
    class Reader final {
      public:
        using value_type = line_info;
        friend class iterator<Reader>;
        Reader() = default;
        ~Reader();

        void open(const std::string &filename);
        void from_string(const std::string &code);
        // bool set_encoding(const std::string& encoding);
        std::string get_filename() const;
        size_t size() const { return _lines.size(); }
        line_info &operator[](const size_t index) const {
            return *(_lines[index]);
        }
        iterator<Reader> begin() { return iterator<Reader>(0, *this); }
        iterator<Reader> end() {
            return iterator<Reader>(_lines.size() - 1, *this);
        }

      private:
        std::string _filename;
        std::vector<line_info *> _lines;
    };

} // namespace tanlang

#endif // TAN_READER_READER_H
