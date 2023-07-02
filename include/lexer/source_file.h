#ifndef TAN_READER_READER_H
#define TAN_READER_READER_H
#include "config.h"
#include "base/container.h"

namespace tanlang {

struct Cursor;

class SourceFile final {
public:
  SourceFile() = default;
  void open(const str &filename);
  void from_string(const str &code);

  /// \brief Return the number of lines of code of the current file
  [[nodiscard]] size_t size() const { return _lines.size(); }

  /** \brief Return source at a specific line
   *  \param index line of code starting from 0
   */
  [[nodiscard]] str get_line(size_t index) const;
  [[nodiscard]] const char *get_line_c_str(size_t index) const;
  [[nodiscard]] char at(const Cursor &ptr) const;

  /**
   * \brief Get a substring from start to the end of the current line
   * \param start start of the string, inclusive
   * */
  [[nodiscard]] str substr(const Cursor &start) const;

  /**
   * \brief Get a substring from the source code
   * \param start start of the string, inclusive
   * \param end end of the string, exclusive
   * */
  [[nodiscard]] str substr(const Cursor &start, const Cursor &end) const;

  /**
   * \brief Check if a cursor is in bound
   */
  [[nodiscard]] bool is_cursor_valid(const Cursor &c) const;

  [[nodiscard]] Cursor begin() const;
  [[nodiscard]] Cursor end() const;

  /// \brief Return a copy of code_ptr that points to the next character
  [[nodiscard]] Cursor forward(Cursor c);

  [[nodiscard]] str get_filename() const;

private:
  vector<str> _lines{};
  str _filename = "memory";
};

struct Cursor {
public:
  friend class SourceFile;
  friend class SourceSpan;
  uint32_t l = 0;
  uint32_t c = 0;

public:
  Cursor(uint32_t r, uint32_t c, const SourceFile *src);

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
  SourceFile *_src = nullptr;
};

/**
 * \brief A span of source code tokens, inclusive on both ends.
 */
class SourceSpan {
public:
  SourceSpan() = delete;
  SourceSpan(const Cursor &start, const Cursor &end);
  [[nodiscard]] SourceFile *src() const;
  [[nodiscard]] const Cursor &start() const;
  [[nodiscard]] const Cursor &end() const;

private:
  const Cursor &_start;
  const Cursor &_end;
};

} // namespace tanlang

#endif // TAN_READER_READER_H
