#include "reader.h"
#include "base.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace tanlang {

    Reader::~Reader() {
        for (auto *&t : _lines) {
            if (t != nullptr) {
                delete t;
                t = nullptr;
            }
        }
    }

    // TODO: optimise Reader for speed
    void Reader::open(const std::string &filename) {
        _filename = filename;
        std::ifstream ifs(filename);
        // read the whole file at once
        std::string content((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        // count the number of lines
        const size_t n_lines = static_cast<size_t>(
            std::count(content.begin(), content.end(), '\n'));
        // reserve memory ahead
        _lines.reserve(n_lines);

        // read line by line
        std::string line;
        unsigned lineno = 0;
        size_t line_start = 0;
        line_info *new_line = nullptr;
        // TODO: check file attributes before reading
        for (size_t c = 0; c < content.length(); ++c) {
            if (content[c] == '\n') {
                line = content.substr(line_start,
                                      c - line_start); // not including '\n'
                // FIXME: avoid `new` inside a loop
                new_line = new line_info(lineno++);
                // delete whitespace at the beginning of the line
                for (size_t i = 0; i < line.length(); ++i) {
                    if (line[i] != '\n' && line[i] != '\r' && line[i] != ' ') {
                        new_line->code =
                            std::string(line.begin() + (long)i, line.end());
                        break;
                    }
                }
                _lines.emplace_back(new_line);
                line_start = c + 1;
            }
        }
    }

    std::string Reader::get_filename() const { return _filename; }

    void Reader::from_string(const std::string &code) {
        _filename = "";
        std::string line;
        unsigned lineno = 0;
        size_t line_start = 0;
        line_info *new_line = nullptr;
        // TODO: check file attributes before reading
        for (size_t c = 0; c < code.length(); ++c) {
            if (code[c] == '\n') {
                line = code.substr(line_start,
                                   c - line_start); // not including '\n'
                // FIXME: avoid `new` inside a loop
                new_line = new line_info(lineno++);
                // delete whitespace at the beginning of the line
                for (size_t i = 0; i < line.length(); ++i) {
                    if (line[i] != '\n' && line[i] != '\r' && line[i] != ' ') {
                        new_line->code =
                            std::string(line.begin() + (long)i, line.end());
                        break;
                    }
                }
                _lines.emplace_back(new_line);
                line_start = c + 1;
            }
        }
    }

} // namespace tanlang
