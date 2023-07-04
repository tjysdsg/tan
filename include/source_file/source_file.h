#ifndef TAN_READER_READER_H
#define TAN_READER_READER_H
#include "config.h"
#include "base/container.h"

namespace tanlang {

class SrcLoc;

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
  [[nodiscard]] char at(const SrcLoc &ptr) const;

  /**
   * \brief Get a substring from start to the end of the current line
   * \param start start of the string, inclusive
   * */
  [[nodiscard]] str substr(const SrcLoc &start) const;

  /**
   * \brief Get a substring from the source code
   * \param start start of the string, inclusive
   * \param end end of the string, exclusive
   * */
  [[nodiscard]] str substr(const SrcLoc &start, const SrcLoc &end) const;

  /**
   * \brief Check if a cursor is in bound
   */
  [[nodiscard]] bool is_cursor_valid(const SrcLoc &c) const;

  /**
   * \brief The start of source file (inclusive).
   */
  [[nodiscard]] SrcLoc begin() const;

  /**
   * \brief The end of source file (exclusive).
   */
  [[nodiscard]] SrcLoc end() const;

  /// \brief Return a copy of code_ptr that points to the next character
  [[nodiscard]] SrcLoc forward(SrcLoc c);

  [[nodiscard]] str get_filename() const;

private:
  vector<str> _lines{};
  str _filename = "memory";
};

class SrcLoc {
public:
  friend class SourceFile;
  friend class SourceSpan;
  uint32_t l = 0;
  uint32_t c = 0;

public:
  SrcLoc(uint32_t r, uint32_t c, const SourceFile *src);

public:
  SrcLoc() = delete;
  SrcLoc &operator=(const SrcLoc &other) = default;
  SrcLoc(const SrcLoc &other) = default;
  ~SrcLoc() = default;
  bool operator==(const SrcLoc &other) const;
  bool operator!=(const SrcLoc &other) const;
  bool operator<=(const SrcLoc &other) const;
  bool operator<(const SrcLoc &other) const;
  bool operator>(const SrcLoc &other) const;
  // prefix increment
  SrcLoc &operator++();
  // postfix increment
  SrcLoc operator++(int);
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
  SourceSpan(const SourceFile *src, uint32_t line, uint32_t col);
  SourceSpan(const SourceFile *src, uint32_t start_line, uint32_t start_col, uint32_t end_line, uint32_t end_col);
  SourceSpan(const SrcLoc &start, const SrcLoc &end);
  [[nodiscard]] SourceFile *src() const;
  [[nodiscard]] const SrcLoc &start() const;
  [[nodiscard]] const SrcLoc &end() const;

private:
  SrcLoc _start;
  SrcLoc _end;
};

} // namespace tanlang

#endif // TAN_READER_READER_H
