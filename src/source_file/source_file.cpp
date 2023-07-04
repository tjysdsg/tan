#include <fstream>
#include "base.h"
#include "source_file/source_file.h"

namespace tanlang {

void SourceFile::open(const str &filename) {
  std::ifstream ifs;
  ifs.open(filename, std::ios::in);
  if (!ifs) {
    Error(ErrorType::FILE_NOT_FOUND, "Cannot open file: " + filename).raise();
  }
  _filename = filename;

  // read the whole file at once
  str content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

  from_string(content);
}

void SourceFile::from_string(const str &code) {
  size_t line_start = 0;
  size_t code_len = code.length();
  for (size_t c = 0; c < code_len; ++c) {
    if (code[c] == '\n' || c == code.length() - 1) {
      str line;
      if (code[c] == '\n') {
        line = code.substr(line_start, c - line_start); // not including trailing '\n'
      } else {
        line = code.substr(line_start, c - line_start + 1);
      }

      // delete whitespace at the beginning of the line
      for (size_t i = 0; i < line.length(); ++i) {
        if (!isspace(line[i])) { // avoid confusion with isspace in <locale>
          line = line.substr(i);
          break;
        }
      }

      _lines.push_back(line);
      line_start = c + 1;
    }
  }
}

bool SourceFile::is_cursor_valid(const SrcLoc &c) const {
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

str SourceFile::substr(const SrcLoc &start, const SrcLoc &_end) const {
  TAN_ASSERT(is_cursor_valid(start));

  /// if the end cursor is out of boundary, make it point to EOF
  SrcLoc end(_end);
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

str SourceFile::substr(const SrcLoc &start) const {
  SrcLoc end(start.l, (uint32_t)get_line(start.l).size(), this);
  return substr(start, end);
}

SrcLoc SourceFile::forward(SrcLoc ptr) {
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

SrcLoc SourceFile::end() const {
  if (_lines.empty()) {
    return SrcLoc(0, 1, this);
  }
  return SrcLoc((uint32_t)_lines.size() - 1, (uint32_t)_lines.back().length(), this);
}

char SourceFile::at(const SrcLoc &ptr) const {
  TAN_ASSERT(ptr.l != -1u && ptr.c != -1u);
  if (ptr.l >= this->size()) {
    return '\0';
  }
  if (ptr.c >= this->_lines[ptr.l].length()) {
    return '\0';
  }
  return _lines[ptr.l][ptr.c];
}

SrcLoc SourceFile::begin() const { return SrcLoc(0, 0, this); }

str SourceFile::get_line(size_t index) const {
  TAN_ASSERT(index < _lines.size());
  return _lines[index];
}

SrcLoc::SrcLoc(uint32_t r, uint32_t c, const SourceFile *src) : l(r), c(c), _src((SourceFile *)src) {}

bool SrcLoc::operator==(const SrcLoc &other) const { return l == other.l && c == other.c; }

bool SrcLoc::operator!=(const SrcLoc &other) const { return !(*this == other); }

bool SrcLoc::operator<=(const SrcLoc &other) const {
  if (l < other.l) {
    return true;
  } else if (l > other.l) {
    return false;
  } else {
    return c <= other.c;
  }
}

bool SrcLoc::operator<(const SrcLoc &other) const {
  if (l < other.l) {
    return true;
  } else if (l > other.l) {
    return false;
  } else {
    return c < other.c;
  }
}

bool SrcLoc::operator>(const SrcLoc &other) const {
  if (l > other.l) {
    return true;
  } else if (l < other.l) {
    return false;
  } else {
    return c > other.c;
  }
}

SrcLoc &SrcLoc::operator++() {
  *this = _src->forward(*this);
  return *this;
}

SrcLoc SrcLoc::operator++(int) {
  auto ret = *this;
  *this = _src->forward(*this);
  return ret;
}

char SrcLoc::operator*() {
  TAN_ASSERT(_src);
  return _src->at(*this);
}

SourceSpan::SourceSpan(const SourceFile *src, uint32_t start_line, uint32_t start_col, uint32_t end_line,
                       uint32_t end_col)
    : SourceSpan(SrcLoc(start_line, start_col, src), SrcLoc(end_line, end_col, src)) {}

SourceSpan::SourceSpan(const SourceFile *src, uint32_t line, uint32_t col) : SourceSpan(src, line, col, line, col) {}

SourceSpan::SourceSpan(const SrcLoc &start, const SrcLoc &end) : _start(start), _end(end) {
  TAN_ASSERT(start._src == end._src);
  TAN_ASSERT(start.l <= end.l);
  if (start.l == end.l) {
    TAN_ASSERT(start.c <= end.c);
  }
}

SourceFile *SourceSpan::src() const { return _start._src; }

const SrcLoc &SourceSpan::start() const { return _start; }

const SrcLoc &SourceSpan::end() const { return _end; }

} // namespace tanlang
