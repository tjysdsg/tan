#ifndef __TAN_SRC_BASE_UTILS_H__
#define __TAN_SRC_BASE_UTILS_H__
#include <vector>
#include <string>

inline std::vector<std::string> flag_to_list(const std::string &flag, const std::string delimiter = ",") {
  size_t last = 0;
  size_t next = 0;
  std::vector<std::string> results;
  while ((next = flag.find(delimiter, last)) != std::string::npos) {
    results.push_back(flag.substr(last, next - last));
    last = next + 1;
  }
  results.push_back(flag.substr(last));
  return results;
}

#endif
