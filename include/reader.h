#ifndef TAN_READER_READER_H
#define TAN_READER_READER_H
#include "config.h"
#include "base.h"
#include <cstdint>
#include <string>
#include <unistd.h>
#include <vector>

namespace tanlang {

struct cursor;

struct line_info {
  size_t lineno;
  std::string code;
  line_info() = delete;
  ~line_info() = default;
  line_info(const size_t lineno, const std::string &code) : lineno(lineno), code(code) {}
};

// TODO: support unicode
class Reader final {
public:
  Reader() = default;
  void open(const std::string &filename);
  void from_string(const std::string &code);

  /// \brief Return the number of lines of code of the current file
  [[nodiscard]] size_t size() const { return _lines.size(); }

  /** \brief Return line_info at the (\index + 1) line
   *  \param index line of code starting from 0
   */
  const line_info &get_line(const size_t index) const;
  char at(const cursor &ptr) const;

  /**
   * \brief Get a substring from start to the end of the current line
   * \param start start of the string, inclusive
   * */
  std::string substr(const cursor &start) const;

  /**
   * \brief Get a substring from the source code
   * \param start start of the string, inclusive
   * \param end end of the string, exclusive
   * */
  std::string substr(const cursor &start, cursor end) const;

  [[nodiscard]] cursor begin() const;
  [[nodiscard]] cursor end() const;

  /// \brief Return a copy of code_ptr that points to the next character
  [[nodiscard]] cursor forward(cursor ptr);

private:
  std::vector<line_info *> _lines{};
};

struct cursor {
  friend class Reader;
  size_t l = 0;
  size_t c = 0;

private:
  cursor(size_t r, size_t c, const Reader *reader) : l(r), c(c), _reader(c_cast(Reader *, reader)) {}

public:
  cursor() = delete;
  cursor &operator=(const cursor &other) = default;
  cursor(const cursor &other) = default;
  ~cursor() = default;
  bool operator==(const cursor &other) { return l == other.l && c == other.c; }
  bool operator!=(const cursor &other) { return !(*this == other); }

  bool operator<(const cursor &other) {
    if (l < other.l) {
      return true;
    } else if (l > other.l) {
      return false;
    } else {
      return c < other.c;
    }
  }

  bool operator>(const cursor &other) {
    if (l > other.l) {
      return true;
    } else if (l < other.l) {
      return false;
    } else {
      return c > other.c;
    }
  }

  /// prefix increment
  cursor &operator++() {
    *this = _reader->forward(*this);
    return *this;
  }

  /// postfix increment
  cursor operator++(int) {
    auto ret = *this;
    *this = _reader->forward(*this);
    return ret;
  }

  char operator*() {
    TAN_ASSERT(_reader);
    return _reader->at(*this);
  }

private:
  Reader *_reader = nullptr;
};

} // namespace tanlang

#endif // TAN_READER_READER_H
