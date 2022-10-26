#include "big_integer.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <limits>
#include <ostream>
#include <stdexcept>

static const uint32_t POW_10_BLOCK = 1'000'000'000;
static const uint32_t POW_10_BLOCK_SIZE = 9;

big_integer::big_integer() = default;

big_integer::big_integer(big_integer const& other) = default;

big_integer::big_integer(int a) : is_neg(a < 0) {
  init_big(static_cast<unsigned long long>(a));
}

big_integer::big_integer(unsigned long long a) {
  init_big(a);
}

big_integer::big_integer(long long a) : is_neg(a < 0) {
  init_big(static_cast<unsigned long long>(a));
}

big_integer::big_integer(unsigned int a) {
  init_big(static_cast<unsigned long long>(a));
}

big_integer::big_integer(long a) : is_neg(a < 0) {
  init_big(static_cast<unsigned long long>(a));
}

big_integer::big_integer(unsigned long a) {
  init_big(static_cast<unsigned long long>(a));
}

big_integer::big_integer(std::string const& str) : big_integer() {
  if (str.empty()) {
    throw std::invalid_argument("String has to be non-empty");
  }
  uint64_t i = (str[0] == '-' ? 1 : 0);
  if (str.length() == 1 && i == 1) {
    throw std::invalid_argument("String cannot be just '-'");
  }
  while (i < str.length()) {
    int32_t cur_num = 0;
    uint32_t k = 0;
    for (; k < POW_10_BLOCK_SIZE; k++) {
      if (std::isdigit(str[i]) == 0) {
        throw std::invalid_argument("String has to contain only numbers");
      }
      cur_num *= 10;
      cur_num += str[i] - '0';
      i++;
      if (i >= str.length()) {
        break;
      }
    }

    if (k == POW_10_BLOCK_SIZE) {
      small_mul(POW_10_BLOCK);
    } else {
      uint32_t pow = 10;
      for (; k > 0; k--) {
        pow *= 10;
      }
      small_mul(pow);
    }
    add_int(cur_num);
  }
  if (str[0] == '-') {
    negate();
  }
}

big_integer::~big_integer() = default;

void big_integer::resize(size_t new_size, uint32_t val) {
  if (new_size <= arr.size()) {
    return;
  }
  arr.resize(new_size, val);
}

big_integer& big_integer::operator=(big_integer const& other) {
  if (this == &other) {
    return *this;
  }
  big_integer copy = big_integer(other);
  swap(copy, *this);
  return *this;
}

uint32_t big_integer::get_complement() const {
  return (is_neg ? std::numeric_limits<uint32_t>::max() : 0);
}

big_integer& big_integer::remove_leading() {
  uint32_t to_remove = get_complement();
  while (!arr.empty() && arr.back() == to_remove) {
    arr.pop_back();
  }
  if (arr.empty() && (to_remove > 0)) {
    arr.push_back(to_remove);
  }
  return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
  add_with_func([](uint32_t num) { return num; }, rhs, 0);
  return *this;
}

void big_integer::add_int(int32_t num) {
  auto carry = static_cast<uint32_t>(num);
  uint32_t num_compl = (num < 0 ? std::numeric_limits<uint32_t>::max() : 0);
  for (size_t i = 0; i < arr.size(); i++) {
    uint64_t res = arr[i];
    if (i != 0) {
      res += num_compl;
    }
    res += carry;
    arr[i] = static_cast<uint32_t>(res);
    carry = res >> 32;
    if (carry == 0 && num_compl == 0) {
      break;
    }
  }
  carry = static_cast<uint32_t>(static_cast<uint64_t>(get_complement()) +
                                num_compl + carry);
  if (carry != get_complement()) {
    arr.push_back(carry);
    is_neg = arr.back() >> 31;
  }
  remove_leading();
}

template <typename F>
void big_integer::add_with_func(F func, big_integer const& rhs,
                                uint32_t carry) {
  size_t new_size = std::max(arr.size(), rhs.arr.size());
  resize(new_size, get_complement());
  for (size_t i = 0; i < new_size; i++) {
    uint64_t res = arr[i];
    res += func(i < rhs.arr.size() ? rhs.arr[i] : rhs.get_complement());
    res += carry;
    arr[i] = static_cast<uint32_t>(res);
    carry = res >> 32;
  }
  carry = static_cast<uint32_t>(static_cast<uint64_t>(get_complement()) +
                                func(rhs.get_complement()) + carry);
  if (carry != get_complement()) {
    arr.push_back(carry);
    is_neg = arr.back() >> 31;
  }
  remove_leading();
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
  add_with_func([](uint32_t num) { return ~num; }, rhs, 1);
  return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
  bool to_negate = is_neg ^ rhs.is_neg;
  big_integer& top = (*this);
  big_integer bot = rhs;
  top.absolutify();
  bot.absolutify();
  std::vector<uint32_t> res;
  res.resize(top.arr.size() + bot.arr.size(), 0);
  uint32_t carry = 0;
  for (size_t i = 0; i < top.arr.size(); i++) {
    for (size_t k = 0; k < bot.arr.size() + 1; k++) {
      uint64_t mul = static_cast<uint64_t>(top.arr[i]) *
                         (k != bot.arr.size() ? bot.arr[k] : 0) +
                     carry + res[i + k];
      res[i + k] = static_cast<uint32_t>(mul);
      carry = mul >> 32;
    }
  }
  arr = res;
  if (to_negate) {
    negate();
  }
  return remove_leading();
}

big_integer& big_integer::small_mul(uint32_t rhs) {
  absolutify();
  uint32_t carry = 0;
  for (unsigned int& i : arr) {
    uint64_t res = static_cast<uint64_t>(i) * rhs + carry;
    i = static_cast<uint32_t>(res);
    carry = res >> 32;
  }
  if (carry) {
    arr.push_back(carry);
  }
  return remove_leading();
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
  knut_div(rhs, DivType::Quot);
  return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
  knut_div(rhs, DivType::Remainder);
  return *this;
}

void big_integer::knut_div(const big_integer& rhs, DivType type) {
  if (rhs == 0) {
    throw std::invalid_argument("big_integer division by zero");
  }
  if (rhs == 1) {
    if (type == DivType::Remainder) {
      (*this) = 0;
    }
    return;
  }
  if (&rhs == this) {
    (*this) = type == DivType::Quot ? 1 : 0;
    return;
  }
  if (rhs.arr.size() > arr.size()) {
    if (type == DivType::Quot) {
      *this = 0;
    }
    return;
  }
  bool was_neg = is_neg;
  bool res_neg = (is_neg ^ rhs.is_neg);
  (*this).absolutify();
  big_integer v = rhs;
  v.absolutify();
  size_t n = v.arr.size();
  if (n == 1) {
    uint32_t rem = div_with_rem(v.arr.back());
    if (type == DivType::Remainder) {
      *this = rem;
      if (was_neg) {
        negate();
      }
      return;
    }
    if (res_neg) {
      negate();
    }
    return;
  }
  uint64_t b = std::numeric_limits<uint32_t>::max() + static_cast<uint64_t>(1);
  uint32_t m = arr.size() - n;
  big_integer q;
  q.resize(m + 1, 0);
  auto d = static_cast<uint32_t>(b / (static_cast<uint64_t>(v.arr.back()) + 1));
  small_mul(d);
  v.small_mul(d);
  (*this).resize(m + n + 1, 0);

  for (int64_t j = m; j >= 0; j--) {
    uint32_t q_ = get_trialed_quot(b, j, n, v);
    uint32_t carry_u = 0;
    sub_from_current_prefix(n, v, q_, j, carry_u);
    q.arr[j] = q_;
    sub_q_if_overflows(n, j, carry_u, q, v);
  }
  if (type == DivType::Quot) {
    if (res_neg) {
      q.negate();
    }
    (*this) = q;
  } else {
    div_with_rem(d);
    if (was_neg) {
      negate();
    }
  }
  remove_leading();
}

void big_integer::sub_q_if_overflows(size_t n, int64_t j, uint32_t carry_u,
                                     big_integer& q, big_integer& v) {
  if (carry_u > 0) {
    q.arr[j]--;
    carry_u = 0;
    for (size_t i = 0; i <= n; i++) {
      uint64_t cur = (i != n ? v.arr[i] : 0) + carry_u + arr[j + i];
      carry_u = cur >> 32;
      arr[j + i] = static_cast<uint32_t>(cur);
    }
  }
}

void big_integer::sub_from_current_prefix(size_t n, big_integer& v, uint32_t q_,
                                          int64_t j, uint32_t& carry_u) {
  uint32_t carry_v = 0;
  for (size_t i = 0; i <= n; i++) {
    uint64_t cur_v =
        (i != n ? v.arr[i] : 0) * static_cast<uint64_t>(q_) + carry_v + carry_u;
    carry_v = cur_v >> 32;
    auto actual = static_cast<uint32_t>(cur_v);
    if (arr[j + i] < actual) {
      carry_u = 1;
    } else {
      carry_u = 0;
    }
    arr[j + i] -= actual;
  }
}

uint32_t big_integer::get_trialed_quot(uint64_t b, int64_t j, size_t n,
                                       big_integer& v) {
  uint32_t q_ =
      (static_cast<uint64_t>(arr[j + n]) * b + arr[j + n - 1]) / v.arr.back();
  uint64_t r_ =
      (static_cast<uint64_t>(arr[j + n]) * b + arr[j + n - 1]) % v.arr.back();
  if (q_ == b ||
      static_cast<uint64_t>(q_) * v.arr[n - 2] > (b * r_ + arr[j + n - 2])) {
    q_--;
    r_ += v.arr[n - 1];
    if (r_ < b) {
      if (q_ == b || static_cast<uint64_t>(q_) * v.arr[n - 2] >
                         (b * r_ + arr[j + n - 2])) {
        q_--;
      }
    }
  }
  return q_;
}

template <typename F>
void big_integer::abstract_bit_operation(F func, const big_integer& rhs) {
  size_t new_size = std::max(arr.size(), rhs.arr.size());
  resize(new_size, get_complement());
  for (size_t i = 0; i < new_size; i++) {
    arr[i] =
        func(arr[i], (i < rhs.arr.size() ? rhs.arr[i] : rhs.get_complement()));
  }
  is_neg = func(is_neg, rhs.is_neg) > 0;
  remove_leading();
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
  abstract_bit_operation([](int a, int b) { return a & b; }, rhs);
  return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
  abstract_bit_operation([](int a, int b) { return a | b; }, rhs);
  return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
  abstract_bit_operation([](int a, int b) { return a ^ b; }, rhs);
  return *this;
}

big_integer& big_integer::operator<<=(int rhs) {
  int offset = rhs / 32;
  int rem = rhs % 32;
  size_t new_size = arr.size() + offset + 1;
  resize(new_size, 0);
  uint32_t carry = get_complement();
  for (int64_t i = new_size - 1; i > offset; i--) {
    uint64_t res = arr[i - offset - 1];
    res >>= 32 - rem;
    res += (carry << rem);
    arr[i] = static_cast<uint32_t>(res);
    carry = arr[i - offset - 1];
  }
  arr[offset] = carry << rem;
  for (int64_t i = offset - 1; i >= 0; i--) {
    arr[i] = 0;
  }
  return remove_leading();
}

big_integer& big_integer::operator>>=(int rhs) {
  if (rhs >= 32 * arr.size()) {
    arr.erase(arr.begin(), arr.end());
    arr.push_back(get_complement());
  } else {
    int offset = rhs / 32;
    int rem = rhs % 32;
    size_t new_size = arr.size() - offset;
    for (uint32_t i = 0; i < new_size - 1; i++) {
      arr[i] = arr[i + offset] >> rem;
      arr[i] += (arr[i + offset + 1] % (1 << rem)) << (32 - rem);
    }
    arr[new_size - 1] = arr.back() >> rem;
    arr[new_size - 1] += (get_complement() % (1 << rem)) << (32 - rem);
    for (uint32_t i = new_size; i < arr.size(); i++) {
      arr[i] = get_complement();
    }
  }
  return remove_leading();
}

big_integer big_integer::operator+() const {
  return *this;
}

big_integer big_integer::operator-() const {
  if (*this == 0) {
    return *this;
  }
  big_integer copy = *this;
  invert_all(copy);
  copy.add_int(1);
  return copy;
}

void big_integer::invert_all(big_integer& a) {
  for (uint32_t& num : a.arr) {
    num = ~num;
  }
  a.is_neg = !a.is_neg;
}

big_integer big_integer::operator~() const {
  big_integer res = *this;
  invert_all(res);
  res.remove_leading();
  return res;
}

big_integer& big_integer::operator++() {
  add_int(1);
  return (*this);
}

big_integer big_integer::operator++(int) {
  big_integer res = *this;
  ++(*this);
  return res;
}

big_integer& big_integer::operator--() {
  add_int(-1);
  return (*this);
}

big_integer big_integer::operator--(int) {
  big_integer res = *this;
  --(*this);
  return res;
}

big_integer operator+(big_integer a, big_integer const& b) {
  return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
  return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
  return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
  return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
  return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b) {
  return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
  return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
  return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
  return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
  return a >>= b;
}

void big_integer::init_big(unsigned long long a) {
  while (a > 0) {
    arr.push_back(static_cast<uint32_t>(a));
    a >>= 32;
  }
  remove_leading();
}

bool operator==(big_integer const& a, big_integer const& b) {
  return (a.is_neg == b.is_neg && a.arr == b.arr);
}

bool operator!=(big_integer const& a, big_integer const& b) {
  return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
  return b > a;
}

bool operator>(big_integer const& a, big_integer const& b) {
  return !(a <= b);
}

bool operator<=(big_integer const& a, big_integer const& b) {
  if (a.is_neg != b.is_neg) {
    return a.is_neg;
  }
  size_t a_sz = a.arr.size();
  size_t b_sz = b.arr.size();
  if (a_sz < b_sz) {
    return !a.is_neg;
  } else if (a_sz > b_sz) {
    return a.is_neg;
  }
  for (size_t i = a_sz; i >= 1; i--) {
    if (a.arr[i - 1] != b.arr[i - 1]) {
      return (a.arr[i - 1] <= b.arr[i - 1]);
    }
  }
  return true;
}

bool operator>=(big_integer const& a, big_integer const& b) {
  return b <= a;
}

void big_integer::negate() {
  invert_all(*this);
  add_int(1);
}

void big_integer::absolutify() {
  if (is_neg) {
    negate();
  }
}

// *this gets divided, returns remainder;
uint32_t big_integer::div_with_rem(uint32_t num) {
  uint32_t rem = 0;
  bool prev_neg = is_neg;
  absolutify();
  size_t index = arr.size();
  size_t last_index = index - 1;
  while (index >= 1) {
    uint64_t res = (static_cast<uint64_t>(rem) << 32) + arr[index - 1];
    if (last_index != arr.size() - 1 || res >= num) {
      arr[last_index--] = res / num;
    }
    rem = res % num;
    index--;
  }
  for (size_t i = last_index + 1; i < arr.size(); i++) {
    std::swap(arr[i], arr[i - last_index - 1]);
  }
  for (size_t i = arr.size() - last_index - 1; i < arr.size(); i++) {
    arr[i] = 0;
  }
  if (prev_neg) {
    negate();
  }
  remove_leading();
  return rem;
}

std::string to_string(big_integer const& a) {
  if (a.arr.empty()) {
    return "0";
  }
  std::string res;
  big_integer copy = a;
  copy.absolutify();
  while (true) {
    uint32_t rem = copy.div_with_rem(POW_10_BLOCK);
    auto rem_len = static_cast<uint32_t>(log10(std::max(rem, 1u)) + 1);
    res += std::to_string(rem);
    std::string::iterator pos = res.begin();
    std::advance(pos, res.length() - rem_len);
    std::reverse(pos, res.end());
    if (copy == 0) {
      break;
    }
    res.append(POW_10_BLOCK_SIZE - rem_len, '0');
  }
  if (a.is_neg) {
    res += '-';
  }
  std::reverse(res.begin(), res.end());
  return res;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
  return s << to_string(a);
}

void swap(big_integer& a, big_integer& b) {
  std::swap(a.arr, b.arr);
  std::swap(a.is_neg, b.is_neg);
}
