#ifndef __TAN_INCLUDE_LINKER_H__
#define __TAN_INCLUDE_LINKER_H__
#include <vector>
#include <string>
#include "base.h"

namespace tanlang {

class Linker final {
public:
  Linker();
  ~Linker() = default;
  void add_files(std::vector<str> filenames);
  void add_flag(str flag);
  void add_flags(std::vector<str> flags);
  bool link();

private:
  std::vector<str> _input_files{};
  std::vector<str> _flags{};
  str _exe = "";
  // TODO: platform, architecture, etc.
};

} // namespace tanlang

#endif /* __TAN_INCLUDE_LINKER_H__ */
