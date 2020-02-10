#include "base.h"
#include "reader.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace tanlang {

Reader::~Reader() {
  for (auto &_line : _lines) {
    if (_line) {
      delete _line;
      _line = nullptr;
    }
  }
}

// TODO: optimise Reader for speed
void Reader::open(const std::string &filename) {
  _filename = filename;
  std::ifstream ifs;
  ifs.open(filename, std::ios::in);
  if (!ifs) {
    throw std::runtime_error("Cannot open file: " + filename);
  }
  // read the whole file at once
  std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  // count the number of lines
  const size_t n_lines = static_cast<size_t>(std::count(content.begin(), content.end(), '\n'));
  // reserve memory ahead
  _lines.reserve(n_lines);
  from_string(content);
}

std::string Reader::get_filename() const { return _filename; }

void Reader::from_string(const std::string &code) {
  std::string line;
  unsigned lineno = 0;
  size_t line_start = 0;
  line_info *new_line = nullptr;
  for (size_t c = 0; c < code.length(); ++c) {
    if (code[c] == '\n' || c == code.length() - 1) {
      if (code[c] == '\n') {
        line = code.substr(line_start,
                           c - line_start); // not including trailing '\n'
      } else {
        line = code.substr(line_start, c - line_start + 1);
      }
      // FIXME: avoid `new` inside a loop
      new_line = new line_info(lineno++, line);
      // delete whitespace at the beginning of the line
      for (size_t i = 0; i < line.length(); ++i) {
        if (!std::isspace(line[i])) {
          new_line->code = std::string(line.begin() + (long) i, line.end());
          break;
        }
      }
      _lines.emplace_back(new_line);
      line_start = c + 1;
    }
  }
}

} // namespace tanlang
