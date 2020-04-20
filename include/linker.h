#ifndef __TAN_INCLUDE_LINKER_H__
#define __TAN_INCLUDE_LINKER_H__
#include <vector>
#include <string>

namespace tanlang {

class Linker final {
public:
  Linker();
  ~Linker() = default;

  void add_files(std::vector<std::string> filenames);
  void add_flag(std::string flag);
  void add_flags(std::vector<std::string> flags);
  bool link();

private:
  std::vector<std::string> _input_files{};
  std::vector<std::string> _flags{};
  std::string _exe = "";
  // TODO: platform, architecture, etc.
};

} // namespace tanlang

#endif /* __TAN_INCLUDE_LINKER_H__ */
