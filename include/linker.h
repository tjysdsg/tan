#ifndef __TAN_INCLUDE_LINKER_H__
#define __TAN_INCLUDE_LINKER_H__
#include <vector>
#include <string>

namespace tanlang {

class Linker {
public:
  Linker() = default;
  ~Linker() = default;

  void add_file(std::string filename);
  void add_files(std::vector<std::string> filenames);
  void add_flag(std::string flag);
  void add_flags(std::vector<std::string> flags);
  void add_flags(int c, char **flags);
  bool link();

private:
  std::vector<std::string> _input_files{};
  std::vector<std::string> _flags{};
  // TODO: output type, platform, architecture, etc.
};
} // namespace tanlang

#endif /* __TAN_INCLUDE_LINKER_H__ */
