#include "reader.h"
#include <fstream>
#include <stdint.h>

namespace tanlang {

struct line_info {
    unsigned lineno = 0;
    std::string code;
    line_info() = default;
    line_info(const unsigned lineno) { this->lineno = lineno; }
};

Reader::~Reader() {
    for (auto*& t : _lines) {
        if (t != nullptr) {
            delete t;
            t = nullptr;
        }
    }
}

void Reader::open(const std::string& file_name) {
    _filename = file_name;
    std::ifstream ifs(file_name);
    std::string line;
    unsigned lineno = 0;
    while (std::getline(ifs, line)) {
        // FIXME: don't malloc in the while loop
        auto* new_line = new line_info(lineno++);
        // delete whitespace at the beginning of the line
        for (int i = 0; i < line.length(); ++i) {
            if (line[i] != '\n' && line[i] != '\r' &&
                line[i] != ' ') {
                new_line->code = std::string(line.begin() + i, line.end());
                break;
            }
        }
        // FIXME: _lines.reserve outside this loop
        _lines.push_back(new_line);
    }
}

std::string Reader::next_line() const {
    if (++_curr_line > _lines.size()) return std::string();
    return (_lines[_curr_line - 1])->code;
}

std::string Reader::get_line(unsigned idx) const {
    _curr_line = idx;
    if (_curr_line - 1 >= _lines.size()) return std::string();
    return (_lines[_curr_line - 1])->code;
}

bool Reader::eof() const { return _curr_line >= _lines.size(); }

std::string Reader::get_filename() const { return _filename; }

unsigned Reader::get_line_number() const { return _curr_line; }

}  // namespace tanlang
