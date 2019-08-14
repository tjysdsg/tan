#ifndef TAN_INCLUDE_UTILS_HPP
#define TAN_INCLUDE_UTILS_HPP
#include <cstddef>
#include <cassert>
#include <tuple>
#include <vector>

namespace tanlang {
    template <typename T> class iterator final {
        size_t index;
        const T &parent;

      public:
        iterator() : index(0), parent(nullptr){};
        explicit iterator(size_t index, const T &parent)
            : index(index), parent(parent){};
        ~iterator() = default;

        // operator overloading
        bool operator==(const iterator<T> &other) {
            // FIXME: check if parents are the same
            return this->index == other.index;
        }

        bool operator!=(const iterator<T> &other) { return !(*this == other); }

        const iterator operator++(int) {
            return iterator<T>(this->index + 1, this->parent);
        }

        iterator &operator++() {
            ++(this->index);
            return *this;
        }

        const typename T::value_type &operator*() const {
            return parent[this->index];
        }
    };

} // namespace tanlang

#endif // TAN_INCLUDE_UTILS_HPP
