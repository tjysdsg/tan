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

// TODO: use raw pointer directly
//  There's no need to allow switching to shared_ptr
template<typename T> using ptr = T *;
template<typename T> inline ptr<T> make_ptr() { return new T; }
template<typename To, typename From> ptr<To> cast_ptr(ptr<From> p) { return dynamic_cast<To *>(p); }
template<typename T, typename... Args> inline ptr<T> make_ptr(Args &&... args) {
  return new T(std::forward<Args>(args)...);
}

#endif //__TAN_SRC_BASE_CONTAINER_H__
