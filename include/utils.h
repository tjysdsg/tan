#ifndef TAN_INCLUDE_UTILS_HPP
#define TAN_INCLUDE_UTILS_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <tuple>
#include <vector>

namespace tanlang {
template<class K, class V, size_t size>
class const_map {
  std::array<std::pair<K, V>, size> data_;

 public:
  template<class... Elements>
  constexpr const_map(Elements... elements) : data_{std::move(elements)...} {}

  constexpr bool contains(const K &key) const {
      return std::find_if(data_.begin(), data_.end(), [&key](const auto &elt) { return elt.first == key; }) !=
          data_.end();
  }

  constexpr const V &operator[](const K &key) const {
      auto it = std::find_if(data_.begin(), data_.end(), [&key](const auto &elt) { return elt.first == key; });
      if (it != data_.end())
          return it->second;
      throw "key not found";
  }
};
} // namespace tanlang

#endif // TAN_INCLUDE_UTILS_HPP
