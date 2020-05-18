#ifndef TAN_READER_READER_H
#define TAN_READER_READER_H
#include "config.h"
#include "base.h"

namespace tanlang {

struct Cursor;

struct SourceLine {
  size_t lineno;
  str code;
  SourceLine() = default;
  ~SourceLine() = default;
  SourceLine(const size_t lineno, const str &code) : lineno(lineno), code(code) {}
};

class Reader final {
public:
  Reader() = default;
  void open(const str &filename);
  void from_string(const str &code);

  /// \brief Return the number of lines of code of the current file
  [[nodiscard]] size_t size() const { return _lines.size(); }

  /** \brief Return source at a specific line
   *  \param index line of code starting from 0
   */
  const SourceLine &get_line(const size_t index) const;
  char at(const Cursor &ptr) const;

  /**
   * \brief Get a substring from start to the end of the current line
   * \param start start of the string, inclusive
   * */
  str substr(const Cursor &start) const;

  /**
   * \brief Get a substring from the source code
   * \param start start of the string, inclusive
   * \param end end of the string, exclusive
   * */
  str substr(const Cursor &start, Cursor end) const;

  [[nodiscard]] Cursor begin() const;
  [[nodiscard]] Cursor end() const;

  /// \brief Return a copy of code_ptr that points to the next character
  [[nodiscard]] Cursor forward(Cursor ptr);

private:
  vector<SourceLine> _lines{};
};

struct Cursor {
  friend class Reader;
  size_t l = 0;
  size_t c = 0;

private:
  Cursor(size_t r, size_t c, const Reader *reader);

public:
  Cursor() = delete;
  Cursor &operator=(const Cursor &other) = default;
  Cursor(const Cursor &other) = default;
  ~Cursor() = default;
  bool operator==(const Cursor &other) const;
  bool operator!=(const Cursor &other) const;
  bool operator<=(const Cursor &other) const;
  bool operator<(const Cursor &other) const;
  bool operator>(const Cursor &other) const;
  // prefix increment
  Cursor &operator++();
  // postfix increment
  Cursor operator++(int);
  char operator*();

private:
  Reader *_reader = nullptr;
};

} // namespace tanlang

#endif // TAN_READER_READER_H
