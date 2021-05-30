#ifndef __TAN_SRC_BASE_CONTAINER_H__
#define __TAN_SRC_BASE_CONTAINER_H__

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <string_view>
#include <optional>

template<typename T> using vector = std::vector<T>;
using str = std::string;
using str_view = std::string_view;
template<typename T1, typename T2> using umap = std::unordered_map<T1, T2>;
template<typename T> using ptr = std::shared_ptr<T>;
template<typename T> using optional = std::optional<T>;

template<typename T, typename... Args> inline ptr<T> make_ptr(Args &&... args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T> using enable_ptr_from_this = std::enable_shared_from_this<T>;

#define ptr_from_this() shared_from_this()

#endif //__TAN_SRC_BASE_CONTAINER_H__
