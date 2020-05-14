#include <sstream>
#include <fstream>
#include <algorithm>
#include "reader.h"
#include "base.h"

namespace tanlang {

void Reader::open(const str &filename) {
  std::ifstream ifs;
  ifs.open(filename, std::ios::in);
  if (!ifs) { throw std::runtime_error("Cannot open file: " + filename); }
  // read the whole file at once
  str content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  // count the number of lines
  size_t n_lines = (size_t) std::count(content.begin(), content.end(), '\n');
  // reserve memory ahead
  _lines.reserve(n_lines);
  from_string(content);
}

void Reader::from_string(const str &code) {
  str line;
  size_t lineno = 0;
  size_t line_start = 0;
  line_info *new_line = nullptr;
  for (size_t c = 0; c < code.length(); ++c) {
    if (code[c] == '\n' || c == code.length() - 1) {
      if (code[c] == '\n') {
        line = code.substr(line_start, c - line_start); // not including trailing '\n'
      } else {
        line = code.substr(line_start, c - line_start + 1);
      }
      // FIXME: avoid `new` inside a loop
      new_line = new line_info(lineno++, line);
      // delete whitespace at the beginning of the line
      for (size_t i = 0; i < line.length(); ++i) {
        if (!std::isspace(line[i])) {
          new_line->code = str(line.begin() + (long) i, line.end());
          break;
        }
      }
      _lines.emplace_back(new_line);
      line_start = c + 1;
    }
  }
}

str Reader::substr(const cursor &start, cursor end) const {
  TAN_ASSERT(start.l != (size_t) -1 && start.c != (size_t) -1);
  // if end can contain -1 only if l and c are both -1
  TAN_ASSERT(!((end.l == (size_t) -1) ^ (end.c == (size_t) -1)));
  if (end.l == (size_t) -1 && end.c == (size_t) -1) {
    end.l = start.l;
    end.c = _lines[end.l]->code.length();
  }
  auto s_row = start.l;
  auto e_row = end.l;
  str ret;
  if (s_row == e_row) {
    TAN_ASSERT(start.c != end.c);
    ret = _lines[s_row]->code.substr(start.c, end.c - start.c);
  } else {
    ret += _lines[s_row]->code.substr(start.c);
    for (auto r = s_row; r < e_row - 1; ++r) {
      ret += _lines[r]->code + "\n";
    }
    if (end.c > 0) {
      ret += _lines[e_row]->code.substr(0, end.c);
    }
  }
  return ret;
}

cursor Reader::forward(cursor ptr) {
  if (ptr.l >= _lines.size()) {
    return ptr;
  }
  size_t n_cols = _lines[ptr.l]->code.length();
  if (ptr.c + 1 >= n_cols) {
    if (ptr.l < _lines.size()) {
      ++ptr.l;
    }
    ptr.c = 0;
  } else {
    ++ptr.c;
  }
  return ptr;
}

cursor Reader::end() const {
  if (_lines.empty()) { return cursor(0, 1, this); }
  return cursor(_lines.size() - 1, _lines.back()->code.length(), this);
}

char Reader::at(const cursor &ptr) const {
  TAN_ASSERT(ptr.l != (size_t) -1 && ptr.c != (size_t) -1);
  if (ptr.l >= this->size()) { return '\0'; }
  if (ptr.c >= this->_lines[ptr.l]->code.length()) { return '\0'; }
  return _lines[ptr.l]->code[ptr.c];
}

str Reader::substr(const cursor &start) const {
  return substr(start, this->end());
}

cursor Reader::begin() const {
  return cursor(0, 0, this);
}

const line_info &Reader::get_line(size_t index) const {
  TAN_ASSERT(index < _lines.size());
  return *(_lines[index]);
}

} // namespace tanlang
