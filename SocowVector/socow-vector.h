#pragma once
#include <array>
#include <cstddef>
#include <memory>

template <typename T, size_t SMALL_SIZE>
struct socow_vector {
  using iterator = T*;
  using const_iterator = T const*;

  socow_vector() {} // = default fails to compile

  socow_vector(socow_vector const& other) {
    if (other.small) {
      copy_raw(get_src(), other.static_arr, other.size_);
    } else {
      small = false;
      dynamic_arr = other.dynamic_arr;
      other.dynamic_arr->inc_ref();
    }
    size_ = other.size_;
  }

  socow_vector& operator=(socow_vector const& other) {
    if (this != &other) {
      socow_vector(other).swap(*this);
    }
    return *this;
  }

  ~socow_vector() {
    destruct_all();
  }

  T& operator[](size_t i) {
    return data()[i];
  }

  T const& operator[](size_t i) const {
    return data()[i];
  }

  T* data() {
    check_ref();
    return get_src();
  }

  T const* data() const {
    return get_src();
  }

  size_t size() const {
    return size_;
  }

  T& front() {
    return data()[0];
  }

  T const& front() const {
    return data()[0];
  }

  T& back() {
    return data()[size_ - 1];
  }

  T const& back() const {
    return data()[size_ - 1];
  }

  void push_back(T const& e) {
    if (size_ == capacity()) {
      size_t new_cap = size_ ? 2 * size_ : 1;
      auto* tmp = copy_raw_with_alloc(get_src(), size_, new_cap);
      try {
        new (tmp->data + size_) T(e);
      } catch (...) {
        destruct_elements(tmp->data, size_);
        operator delete(tmp);
        throw;
      }
      if (small || dynamic_arr->ref_count == 1) {
        destruct_all();
      } else {
        dynamic_arr->dec_ref();
      }
      dynamic_arr = tmp;
      small = false;
    } else {
      check_ref();
      new (get_src() + size_) T(e);
    }
    size_++;
  }

  void pop_back() {
    check_ref();
    size_--;
    get_src()[size_].~T();
  }

  bool empty() const {
    return size_ == 0;
  }

  size_t capacity() const {
    return small ? SMALL_SIZE : dynamic_arr->capacity;
  }

  void reserve(size_t new_cap) {
    check_ref(std::max(size_, new_cap));
    if (new_cap > capacity()) {
      set_capacity(new_cap);
    }
  }

  void shrink_to_fit() {
    if (!small && size_ != capacity()) {
      if (size_ <= SMALL_SIZE) {
        convert_to_static();
      } else  {
        check_ref(size_);
        if (size_ != capacity()) {
          set_capacity(size_);
        }
      }
    }
  }

  void clear() {
    check_ref();
    destruct_elements(get_src(), size_);
    size_ = 0;
  }

  void swap(socow_vector& other) {
    if (small) {
      if (other.small) {
        size_t min_size = std::min(size_, other.size_);
        if (size_ == min_size) {
          transfer_static_overhead(static_arr, other.static_arr, other.size_,
                                   min_size);
        } else {
          transfer_static_overhead(other.static_arr, static_arr, size_,
                                   min_size);
        }
        for (size_t i = 0; i < min_size; i++) {
          std::swap(static_arr[i], other.static_arr[i]);
        }
        std::swap(small, other.small);
        std::swap(size_, other.size_);
      } else {
        swap_dynamic_static(other, *this);
      }
    } else {
      if (other.small) {
        swap_dynamic_static(*this, other);
      } else {
        std::swap(dynamic_arr, other.dynamic_arr);
        std::swap(size_, other.size_);
      }
    }
  }

  iterator begin() {
    return data();
  }

  iterator end() {
    return data() + size_;
  }

  const_iterator begin() const {
    return data();
  }

  const_iterator end() const {
    return data() + size_;
  }

  iterator insert(const_iterator pos, T const& e) {
    ptrdiff_t end_pos = pos - get_src();
    push_back(e);
    for (ptrdiff_t i = end() - begin() - 1; i > end_pos; i--) {
      std::swap(get_src()[i], get_src()[i - 1]);
    }
    return begin() + end_pos;
  }

  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, const_iterator last) {
    ptrdiff_t erased_pos = first - get_src();
    ptrdiff_t pos_dif = last - first;
    check_ref();
    T* src = get_src();
    for (ptrdiff_t i = erased_pos + pos_dif; i < size_; i++) {
      std::swap(src[i], src[i - pos_dif]);
    }
    for (size_t i = 0; i < pos_dif; i++) {
      pop_back();
    }
    return src + erased_pos;
  }

private:
  struct dynamic_buf {
    size_t capacity;
    size_t ref_count{1};
    T data[0];

    void inc_ref() {
      ++ref_count;
    }
    void dec_ref() {
      --ref_count;
    }
  };
  bool small{true};
  size_t size_{0};

  union {
    T static_arr[SMALL_SIZE];
    dynamic_buf* dynamic_arr;
  };

  static void copy_raw(T* dst, const T* src, size_t size) {
    copy_raw(dst, src, size, 0);
  }

  static void copy_raw(T* dst, const T* src, size_t size, size_t begin) {
    size_t i = begin;
    try {
      for (; i < size; i++) {
        new (dst + i) T(src[i]);
      }
    } catch (...) {
      destruct_elements(dst, i, begin);
      throw;
    }
  }

  static dynamic_buf* copy_raw_with_alloc(const T* src, size_t size,
                                          size_t new_cap) {
    size_t bytes = sizeof(dynamic_buf) + new_cap * sizeof(T);
    void* buffer = operator new(bytes);
    auto* tmp = new (buffer) dynamic_buf{new_cap};
    try {
      copy_raw(tmp->data, src, size);
    } catch (...) {
      operator delete(tmp);
      throw;
    }
    return tmp;
  }

  static void destruct_elements(T* dst, size_t end) {
    std::destroy_n(dst, end);
    //destruct_elements(dst, end, 0);
  }

  static void destruct_elements(T* dst, size_t end, size_t begin) {
    for (size_t i = end; i > begin; i--) {
      dst[i - 1].~T();
    }
  }

  // left is always dynamic, right is static
  static void swap_dynamic_static(socow_vector& dynamic_vec,
                                  socow_vector& static_vec) {
    auto* tmp = dynamic_vec.dynamic_arr;
    try {
      copy_raw(dynamic_vec.static_arr, static_vec.static_arr, static_vec.size_);
    } catch (...) {
      dynamic_vec.dynamic_arr = tmp;
      throw;
    }
    static_vec.destruct_all();
    static_vec.dynamic_arr = tmp;
    std::swap(dynamic_vec.size_, static_vec.size_);
    std::swap(dynamic_vec.small, static_vec.small);
  }

  static void transfer_static_overhead(T* dst, T* src, size_t size,
                                       size_t begin) {
    copy_raw(dst, src, size, begin);
    destruct_elements(src, size, begin);
  }

  T* get_src() {
    return small ? static_arr : dynamic_arr->data;
  }

  T const* get_src() const {
    return small ? static_arr : dynamic_arr->data;
  }

  void destruct_all() {
    if (small || dynamic_arr->ref_count == 1) {
      destruct_elements(get_src(), size_);
      if (!small && dynamic_arr) {
        operator delete(dynamic_arr);
      }
    } else {
      dynamic_arr->dec_ref();
    }
  }

  void set_capacity(size_t new_cap) {
    auto* tmp = copy_raw_with_alloc(get_src(), size_, new_cap);
    destruct_all();
    dynamic_arr = tmp;
    small = false;
  }

  void convert_to_static() {
    auto* tmp = dynamic_arr;
    try {
      copy_raw(static_arr, tmp->data, size_);
    } catch (...) {
      dynamic_arr = tmp;
      throw;
    }
    if(tmp->ref_count == 1) {
      destruct_elements(tmp->data, size_);
      operator delete(tmp);
    } else {
      tmp->dec_ref();
    }
    small = true;
  }


  void check_ref() {
    check_ref(capacity());
  }

  void check_ref(size_t new_cap) {
    if (small || dynamic_arr->ref_count == 1) {
      return;
    }
    auto* tmp = copy_raw_with_alloc(dynamic_arr->data, size_, new_cap);
    dynamic_arr->dec_ref();
    dynamic_arr = tmp;
  }
};
