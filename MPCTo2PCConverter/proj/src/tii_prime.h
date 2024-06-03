#ifndef MPCTO2PCCONVERTER_TIIPRIME_H
#define MPCTO2PCCONVERTER_TIIPRIME_H

#include <boost/multiprecision/cpp_int.hpp>
#include <iosfwd>
#include <iostream>

const int __OVERFLOW_GUARD__ = 10'000'000;

using boost::multiprecision::int128_t;
using boost::multiprecision::uint128_t;

class TiiPrime {
private:
  static const uint128_t p;    // 170141183460469231731687303715887185921
  static const int128_t halfP; // 85070591730234615865843651857943592960

  int128_t elem;

public:
  explicit TiiPrime() : elem(0){};
  explicit TiiPrime(const int128_t &num) : elem(num){};
  TiiPrime(const int128_t &num, bool check) : elem(num) {
    if (check == true) {
      if (elem > halfP) {
        elem -= p;
      }
      if (elem < (-halfP)) {
        elem += p;
      }
    }

    if (elem > halfP || elem < -halfP) {
      std::cout << "error in ctor. val is not in the range" << std::endl;
    }
  };

  TiiPrime &operator= (const TiiPrime &other) = default;

  bool operator!=(const TiiPrime &other) const {
    return !(other.elem == elem);
  };
  bool operator==(const TiiPrime &other) const { return other.elem == elem; };

  std::string to_string() const { return elem.str(); }

  TiiPrime operator+(const TiiPrime &f2) const {
    TiiPrime result = *this; // Make a copy of myself.
    result += f2;            // Use += to add f2 to the copy.
    return result;
  }

  TiiPrime operator-(const TiiPrime &f2) const {
    TiiPrime result = *this; // Make a copy of myself.
    result -= f2;            // Use -= to add f2 to the copy.
    return result;
  }

  TiiPrime operator-=(const TiiPrime &f2) {
    bool flagChanged = false;
    // deal with overflow
    // for large positive numbers
    if ((elem > 0) && (halfP - elem < __OVERFLOW_GUARD__) &&
        (halfP + f2.elem < __OVERFLOW_GUARD__)) {
      int128_t temp = elem - __OVERFLOW_GUARD__;

      elem = (temp - f2.elem) - p;
      elem += __OVERFLOW_GUARD__;

      flagChanged = true;
    }
    // for large negative numbers
    else if ((halfP + elem < __OVERFLOW_GUARD__) &&
             (halfP - f2.elem < __OVERFLOW_GUARD__)) {
      int128_t temp = elem + __OVERFLOW_GUARD__;

      elem = (temp - f2.elem) + p;
      elem -= __OVERFLOW_GUARD__;

      flagChanged = true;
    }

    if (flagChanged == false) {
      elem = (elem - f2.elem);
    }

    if (elem > halfP) {
      elem -= p;
    } else if (elem < -halfP)
      elem += p;

    return *this;
  }

  TiiPrime &operator+=(const TiiPrime &f2) {
    bool flagChanged = false;
    // deal with overflow
    // for large positive numbers
    if ((elem > 0) && (halfP - elem < __OVERFLOW_GUARD__) &&
        (halfP + f2.elem < __OVERFLOW_GUARD__)) {
      int128_t temp = elem - __OVERFLOW_GUARD__;

      elem = (temp + f2.elem) - p;
      elem += __OVERFLOW_GUARD__;

      flagChanged = true;
    }
    // for large negative numbers
    else if ((halfP + elem < __OVERFLOW_GUARD__) &&
             (halfP - f2.elem < __OVERFLOW_GUARD__)) {
      int128_t temp = elem + __OVERFLOW_GUARD__;

      elem = (temp + f2.elem) + p;
      elem -= __OVERFLOW_GUARD__;

      flagChanged = true;
    }

    if (flagChanged == false) {
      elem = (elem + f2.elem);
    }

    if (elem > halfP) {
      elem -= p;
    } else if (elem < (-halfP))
      elem += p;

    return *this;
  };
  friend std::ostream &operator<<(std::ostream &os, const TiiPrime &a);
  friend std::istream &operator>>(std::istream &is, TiiPrime &a);
};

inline std::ostream &operator<<(std::ostream &os, const TiiPrime &a) {
  return os << a.elem;
};
inline std::istream &operator>>(std::istream &is, TiiPrime &a) {
  return is >> a.elem;
};

#endif // MPCTO2PCCONVERTER_TIIPRIME_H
