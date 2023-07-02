#include <fstream>
#include <algorithm>
#include "source_file/source_file.h"
#include "base.h"

/// we always assume that the line number and column number can be contained within 32bit int
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

namespace tanlang {

void SourceFile::open(const str &filename) {
  std::ifstream ifs;
  ifs.open(filename, std::ios::in);
  if (!ifs) {
    Error(ErrorType::FILE_NOT_FOUND, "Cannot open file: " + filename).raise();
  }
  _filename = filename;

  /// read the whole file at once
  str content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
  /// count the number of lines
  size_t n_lines = (size_t)std::count(content.begin(), content.end(), '\n');
  /// reserve memory ahead
  _lines.reserve(n_lines);
  from_string(content);
}

void SourceFile::from_string(const str &code) {
  str line;
  size_t line_start = 0;
  str new_line;
  size_t code_len = code.length();
  for (size_t c = 0; c < code_len; ++c) {
    if (code[c] == '\n' || c == code.length() - 1) {
      if (code[c] == '\n') {
        line = code.substr(line_start, c - line_start); /// not including trailing '\n'
      } else {
        line = code.substr(line_start, c - line_start + 1);
      }
      new_line = line;
      /// delete whitespace at the beginning of the line
      for (size_t i = 0; i < line.length(); ++i) {
        if (!isspace(line[i])) { // avoid confusion with isspace in <locale>
          new_line = line.substr(i);
          break;
        }
      }
      _lines.push_back(new_line);
      line_start = c + 1;
    }
  }
}

bool SourceFile::is_cursor_valid(const Cursor &c) const {
  if (c.l >= _lines.size()) {
    return false;
  }

  str line = _lines[c.l];
  if (line.empty()) {
    return c.c == 0;
  } else {
    return c.c < line.size();
  }
}

str SourceFile::substr(const Cursor &start, const Cursor &_end) const {
  TAN_ASSERT(is_cursor_valid(start));

  /// if the end cursor is out of boundary, make it point to EOF
  Cursor end(_end);
  if (end.l >= _lines.size() || end.c > _lines[end.l].size()) {
    end.l = (uint32_t)_lines.size() - 1;
    end.c = (uint32_t)_lines.back().size();
  }
  auto s_row = start.l;
  auto e_row = end.l;
  str ret;
  if (s_row == e_row) {
    TAN_ASSERT(start.c <= end.c);
    ret = _lines[s_row].substr(start.c, end.c - start.c);
  } else {
    ret += _lines[s_row].substr(start.c) + "\n";
    for (auto r = s_row + 1; r < e_row; ++r) {
      ret += _lines[r] + "\n";
    }
    if (end.c > 0) {
      ret += _lines[e_row].substr(0, end.c);
    }
  }
  return ret;
}

str SourceFile::substr(const Cursor &start) const {
  Cursor end(start.l, (uint32_t)get_line(start.l).size(), this);
  return substr(start, end);
}

Cursor SourceFile::forward(Cursor ptr) {
  if (ptr.l >= _lines.size()) {
    return ptr;
  }
  size_t n_cols = _lines[ptr.l].length();
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

str SourceFile::get_filename() const { return _filename; }

Cursor SourceFile::end() const {
  if (_lines.empty()) {
    return Cursor(0, 1, this);
  }
  return Cursor((uint32_t)_lines.size() - 1, (uint32_t)_lines.back().length(), this);
}

char SourceFile::at(const Cursor &ptr) const {
  TAN_ASSERT(ptr.l != -1u && ptr.c != -1u);
  if (ptr.l >= this->size()) {
    return '\0';
  }
  if (ptr.c >= this->_lines[ptr.l].length()) {
    return '\0';
  }
  return _lines[ptr.l][ptr.c];
}

Cursor SourceFile::begin() const { return Cursor(0, 0, this); }

str SourceFile::get_line(size_t index) const {
  TAN_ASSERT(index < _lines.size());
  return _lines[index];
}

const char *SourceFile::get_line_c_str(size_t index) const {
  TAN_ASSERT(index < _lines.size());
  return _lines[index].c_str();
}

Cursor::Cursor(uint32_t r, uint32_t c, const SourceFile *src) : l(r), c(c), _src((SourceFile *)src) {}

bool Cursor::operator==(const Cursor &other) const { return l == other.l && c == other.c; }

bool Cursor::operator!=(const Cursor &other) const { return !(*this == other); }

bool Cursor::operator<=(const Cursor &other) const {
  if (l < other.l) {
    return true;
  } else if (l > other.l) {
    return false;
  } else {
    return c <= other.c;
  }
}

bool Cursor::operator<(const Cursor &other) const {
  if (l < other.l) {
    return true;
  } else if (l > other.l) {
    return false;
  } else {
    return c < other.c;
  }
}

bool Cursor::operator>(const Cursor &other) const {
  if (l > other.l) {
    return true;
  } else if (l < other.l) {
    return false;
  } else {
    return c > other.c;
  }
}

Cursor &Cursor::operator++() {
  *this = _src->forward(*this);
  return *this;
}

Cursor Cursor::operator++(int) {
  auto ret = *this;
  *this = _src->forward(*this);
  return ret;
}

char Cursor::operator*() {
  TAN_ASSERT(_src);
  return _src->at(*this);
}

SourceSpan::SourceSpan(const Cursor &start, const Cursor &end) : _start(start), _end(end) {
  TAN_ASSERT(start._src == end._src);
  TAN_ASSERT(start.l <= end.l);
  if (start.l == end.l) {
    TAN_ASSERT(start.c <= end.c);
  }
}

SourceFile *SourceSpan::src() const { return _start._src; }

const Cursor &SourceSpan::start() const { return _start; }

const Cursor &SourceSpan::end() const { return _end; }

} // namespace tanlang

#pragma clang diagnostic pop