#ifndef __TAN_SRC_BASE_UTILS_H__
#define __TAN_SRC_BASE_UTILS_H__
#include <vector>
#include <string>
#include "src/base/container.h"

inline vector<str> flag_to_list(const str &flag, const str &delimiter = ",") {
  size_t last = 0;
  size_t next = 0;
  vector<str> results;
  while ((next = flag.find(delimiter, last)) != str::npos) {
    results.push_back(flag.substr(last, next - last));
    last = next + 1;
  }
  results.push_back(flag.substr(last));
  return results;
}

#endif
