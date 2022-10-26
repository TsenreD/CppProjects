#pragma once

#include <iosfwd>
#include <string>
#include <vector>

struct big_integer {
  big_integer();
  big_integer(big_integer const& other);
  big_integer(int a);
  big_integer(unsigned int a);
  big_integer(long a);
  big_integer(unsigned long a);
  big_integer(long long a);
  big_integer(unsigned long long a);
  explicit big_integer(std::string const& str);
  ~big_integer();

  big_integer& operator=(big_integer const& other);

  big_integer& operator+=(big_integer const& rhs);
  big_integer& operator-=(big_integer const& rhs);
  big_integer& operator*=(big_integer const& rhs);

  big_integer& operator/=(big_integer const& rhs);
  big_integer& operator%=(big_integer const& rhs);

  big_integer& operator&=(big_integer const& rhs);
  big_integer& operator|=(big_integer const& rhs);
  big_integer& operator^=(big_integer const& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);
  friend bool operator==(big_integer const& a, big_integer const& b);
  friend bool operator!=(big_integer const& a, big_integer const& b);
  friend bool operator<(big_integer const& a, big_integer const& b);
  friend bool operator>(big_integer const& a, big_integer const& b);
  friend bool operator<=(big_integer const& a, big_integer const& b);
  friend bool operator>=(big_integer const& a, big_integer const& b);

  friend std::string to_string(big_integer const& a);
  friend void swap(big_integer& a, big_integer& b);

  void absolutify();

  void negate();

private:
  std::vector<uint32_t> arr;
  bool is_neg{false};
  enum class DivType { Quot, Remainder };

  void add_int(int32_t num);

  static void invert_all(big_integer& a);

  template <typename F>
  void abstract_bit_operation(F func, big_integer const& rhs);

  template <typename F>
  void add_with_func(F func, big_integer const& rhs, uint32_t carry);

  big_integer& remove_leading();

  void init_big(unsigned long long a);

  uint32_t get_complement() const;

  uint32_t div_with_rem(uint32_t num);

  void resize(size_t new_size, uint32_t val);

  void knut_div(big_integer const& rhs, DivType type);

  uint32_t get_trialed_quot(uint64_t b, int64_t j, size_t n, big_integer& v);

  void sub_from_current_prefix(size_t n, big_integer& v, uint32_t q_, int64_t j,
                               uint32_t& carry_u);

  void sub_q_if_overflows(size_t n, int64_t j, uint32_t carry_u, big_integer& q,
                          big_integer& v);

  big_integer& small_mul(uint32_t rhs);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);

bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);
