#ifndef __TAN_SRC_BASE_CONTAINER_H__
#define __TAN_SRC_BASE_CONTAINER_H__

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <functional>

template<typename T> using vector = std::vector<T>;

using str = std::string;

template<typename Key, typename Value, typename Hash = std::hash<Key>> // support custom hash
using umap = std::unordered_map<Key, Value, Hash>;

struct PairHash {
  template<class T1, class T2> std::size_t operator()(const std::pair<T1, T2> &p) const {
    // https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes/27952689#27952689
    size_t lhs = std::hash<T1>{}(p.first);
    size_t rhs = std::hash<T2>{}(p.second);
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
  }
};

template<typename To, typename From> To *cast_ptr(From *p) {
  #ifdef DEBUG
  return dynamic_cast<To *>(p);
  #else
  return reinterpret_cast<To *>(p);
  #endif
}

#endif //__TAN_SRC_BASE_CONTAINER_H__
