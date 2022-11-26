#ifndef __TAN_INCLUDE_LINKER_H__
#define __TAN_INCLUDE_LINKER_H__
#include "base.h"

namespace tanlang {

class Linker final {
public:
  Linker();
  ~Linker() = default;
  void add_files(vector<str> filenames);
  void add_flag(const str &flag);
  void add_flags(vector<str> flags);
  bool link();

private:
  vector<str> _input_files{};
  vector<str> _flags{};
  str _exe = "";
  // TODO: platform, architecture, etc.
};

} // namespace tanlang

#endif /* __TAN_INCLUDE_LINKER_H__ */
