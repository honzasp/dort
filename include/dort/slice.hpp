#pragma once
#include <stdexcept>
#include <vector>

namespace dort {
  template<class T>
  class slice {
    T* ptr;
    size_t len;
  public:
    slice(): slice(nullptr, 0) { }
    slice(const slice&) = default;
    slice(slice&&) = default;
    slice& operator=(const slice&) = default;
    slice& operator=(slice&&) = default;

    slice(T* ptr, size_t len): ptr(ptr), len(len) { }
    slice(T* begin, T* end): ptr(begin), len(end - begin) { }

    template<class Y, class A>
    explicit slice(std::vector<Y, A>& vec): slice(vec.data(), vec.size()) { }
    template<class Y, class A>
    explicit slice(const std::vector<Y, A>& vec): slice(vec.data(), vec.size()) { }

    size_t size() const {
      return this->len;
    }

    bool empty() const {
      return this->len != 0;
    }

    T& at(size_t idx) const {
      if(idx >= this->len) {
        throw std::out_of_range("slice::at");
      }
      return this->ptr[idx];
    }

    T& operator[](size_t idx) const {
      return this->ptr[idx];
    }

    T& front() const {
      if(this->len == 0) {
        throw std::out_of_range("slice::front");
      }
      return this->ptr[0];
    }

    T& back() const {
      if(this->len == 0) {
        throw std::out_of_range("slice::back");
      }
      return this->ptr[this->len - 1];
    }

    T* data() const {
      return this->ptr;
    }

    slice subslice(size_t begin, size_t end) const {
      if(begin > this->len || end > this->len) {
        throw std::out_of_range("slice::subslice");
      }
      return slice(this->ptr + begin, end > begin ? end - begin : 0);
    }

    slice subslice_len(size_t begin, size_t len) const {
      if(begin > this->len || begin + len > this->len) {
        throw std::out_of_range("slice::subslice_len");
      }
      return slice(this->ptr + begin, len);
    }

    slice subslice_from(size_t begin) const {
      if(begin > this->len) {
        throw std::out_of_range("slice::subslice_from");
      }
      return slice(this->ptr + begin, this->len - begin);
    }

    slice subslice_to(size_t end) const {
      if(end > this->len) {
        throw std::out_of_range("slice::subslice_to");
      }
      return slice(this->ptr, end);
    }

    T* begin() const {
      return this->ptr;
    }
    T* end() const {
      return this->ptr + this->len;
    }
  };

  template<class T, class A>
  slice<const T> make_slice(const std::vector<T, A>& vec) {
    return slice<const T>(vec);
  }

  template<class T, class A>
  slice<T> make_slice(std::vector<T, A>& vec) {
    return slice<T>(vec);
  }
}
