#ifndef __TAN_SRC_BASE_CONTAINER_H__
#define __TAN_SRC_BASE_CONTAINER_H__

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <functional>

template<typename T> using vector = std::vector<T>;
using str = std::string;
template<typename T1, typename T2> using umap = std::unordered_map<T1, T2>;

template<typename To, typename From> To *cast_ptr(From *p) {
  #ifdef DEBUG
  return dynamic_cast<To *>(p);
  #else
  return reinterpret_cast<To *>(p);
  #endif
}

#endif //__TAN_SRC_BASE_CONTAINER_H__
