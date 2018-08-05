#pragma once

#include "internal/library.h"

#include <stdexcept>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <type_traits>

namespace yogi {
namespace internal {

template <typename T>
inline bool IsFinite(T) {
  return true;
}

template <>
inline bool IsFinite<float>(float val) {
  return std::isfinite(val);
}

template <>
inline bool IsFinite<double>(double val) {
  return std::isfinite(val);
}

template <typename T>
inline bool IsNan(T) {
  return false;
}

template <>
inline bool IsNan<float>(float val) {
  return std::isnan(val);
}

template <>
inline bool IsNan<double>(double val) {
  return std::isnan(val);
}

enum InfinityType {
  kNegative = -1,
  kNone = 0,
  kPositive = 1,
};

inline long long AddSafely(long long a, long long b) {
  if (a > 0 && b > (std::numeric_limits<long long>::max)() - a) {
    throw std::overflow_error("Duration value overflow.");
  } else if (a < 0 && b < (std::numeric_limits<long long>::min)() - a) {
    throw std::underflow_error("Duration value underflow.");
  }

  return a + b;
}

template <typename T>
inline long long MultiplySafely(long long val, T multiplicator) {
  if (IsNan(multiplicator)) {
    throw std::range_error("Trying to multiply duration value and NaN.");
  }

  if (multiplicator == T{}) {
    return 0;
  }

  long long max_val = static_cast<long long>(
      (std::numeric_limits<long long>::max)() / multiplicator);
  if (std::abs(val) > max_val) {
    throw std::overflow_error("Duration value overflow.");
  }

  return static_cast<long long>(val * multiplicator);
}

template <typename T>
inline long long DivideSafely(long long val, T divisor) {
  if (IsNan(divisor)) {
    throw std::range_error("Trying to divide duration value by NaN.");
  }

  if (divisor == T{}) {
    throw std::range_error("Trying to divide duration value by zero.");
  }

  if (!IsFinite(divisor)) {
    return 0;
  }

  if (std::abs(divisor) < static_cast<T>(1)) {
    long long max_val = static_cast<long long>(
        (std::numeric_limits<long long>::max)() * divisor);
    if (std::abs(val) > max_val) {
      throw std::overflow_error("Duration value overflow.");
    }
  }

  return static_cast<long long>(val / divisor);
}

}  // namespace internal

class Duration {
 public:
  typedef long long value_type;

  static const Duration kInfinity;
  static const Duration kNegativeInfinity;

  static Duration FromNanoseconds(long long ns) {
    return Duration(std::chrono::nanoseconds(ns));
  }

  static Duration FromMicroseconds(long long us) {
    return FromNanoseconds(internal::MultiplySafely(us, 1e3));
  }

  static Duration FromMilliseconds(long long ms) {
    return FromNanoseconds(internal::MultiplySafely(ms, 1e6));
  }

  static Duration FromSeconds(long long seconds) {
    return FromNanoseconds(internal::MultiplySafely(seconds, 1e9));
  }

  static Duration FromMinutes(long long minutes) {
    return FromNanoseconds(internal::MultiplySafely(minutes, 60 * 1e3));
  }

  static Duration FromHours(long long hours) {
    return FromNanoseconds(internal::MultiplySafely(hours, 60 * 60 * 1e3));
  }

  static Duration FromDays(long long days) {
    return FromNanoseconds(internal::MultiplySafely(days, 24 * 60 * 60 * 1e3));
  }

  Duration() {}

  explicit Duration(const std::chrono::nanoseconds& ns)
      : ns_count_(ns.count()) {}

  long long Nanoseconds() const { return ns_count_ % 1000; }

  long long Microseconds() const { return (ns_count_ / 1'000) % 1000; }

  long long Milliseconds() const { return (ns_count_ / 1'000'000) % 1000; }

  long long Seconds() const { return (ns_count_ / 1'000'000'000) % 60; }

  long long Minutes() const { return TotalSeconds() % 60; }

  long long Hours() const { return TotalMinutes() % 24; }

  long long Days() const { return TotalHours() / 24; }

  long long TotalNanoseconds() const { return ns_count_; }

  long long TotalMicroseconds() const { return ns_count_ / 1'000; }

  long long TotalMilliseconds() const { return ns_count_ / 1'000'000; }

  long long TotalSeconds() const { return ns_count_ / 1'000'000'000; }

  long long TotalMinutes() const { return TotalSeconds() / 60; }

  long long TotalHours() const { return TotalMinutes() / 60; }

  long long TotalDays() const { return TotalHours() / 24; }

  Duration Negated() const { return FromNanoseconds(-ns_count_); }

  template <typename T = std::chrono::nanoseconds>
  T ToChronoDuration() const {
    switch (inf_type_) {
      case internal::kPositive:
        return (T::max)();

      case internal::kNegative:
        return (T::min)();

      default:  // internal::kNone
        return std::chrono::duration_cast<T>(
            std::chrono::nanoseconds(ns_count_));
    }
  }

  bool IsInfinity() const { return inf_type_ != internal::kNone; }

  std::string ToString() const {
    // TODO
  }

  Duration operator+(const Duration& rhs) const {
    if (inf_type_ == internal::kNone && rhs.inf_type_ == internal::kNone) {
      return FromNanoseconds(internal::AddSafely(ns_count_, rhs.ns_count_));
    }

    auto inf_type_sum = inf_type_ + rhs.inf_type_;
    if (inf_type_sum == internal::kNone) {
      throw std::range_error(
          "Trying to add positive and negative infinite duration values.");
    }

    return inf_type_sum > 0 ? kInfinity : kNegativeInfinity;
  }

  Duration operator+(const std::chrono::nanoseconds& rhs) const {
    return *this + Duration(rhs);
  }

  Duration operator-(const Duration& rhs) const {
    return *this + rhs.Negated();
  }

  Duration operator-(const std::chrono::nanoseconds& rhs) const {
    return *this - Duration(rhs);
  }

  template <typename T>
  Duration operator*(T rhs) const {
    static_assert(std::is_arithmetic<T>::value,
                  "T must be an arithmetic type.");

    if (inf_type_ != internal::kNone && rhs == T{}) {
      throw std::range_error(
          "Trying to multiply infinite duration value and zero.");
    }

    if (inf_type_ == internal::kNone && internal::IsFinite(rhs)) {
      return FromNanoseconds(internal::MultiplySafely(ns_count_, rhs));
    }

    auto rhs_inf_type = rhs > T{} ? internal::kPositive : internal::kNegative;
    return inf_type_ == rhs_inf_type ? kInfinity : kNegativeInfinity;
  }

  template <typename T>
  Duration operator/(T rhs) const {
    static_assert(std::is_arithmetic<T>::value,
                  "T must be an arithmetic type.");

    if (inf_type_ == internal::kNone) {
      return FromNanoseconds(internal::DivideSafely(ns_count_, rhs));
    }

    auto rhs_inf_type = rhs > T{} ? internal::kPositive : internal::kNegative;
    return inf_type_ == rhs_inf_type ? kInfinity : kNegativeInfinity;
  }

  Duration& operator+=(const Duration& rhs) {
    *this = *this + rhs;
    return *this;
  }

  Duration& operator+=(const std::chrono::nanoseconds& rhs) {
    *this = *this + rhs;
    return *this;
  }

  Duration& operator-=(const Duration& rhs) {
    *this = *this - rhs;
    return *this;
  }

  Duration& operator-=(const std::chrono::nanoseconds& rhs) {
    *this = *this - rhs;
    return *this;
  }

  Duration& operator*=(int rhs) {
    *this = *this * rhs;
    return *this;
  }

  Duration& operator*=(float rhs) {
    *this = *this * rhs;
    return *this;
  }

  Duration& operator/=(int rhs) {
    *this = *this / rhs;
    return *this;
  }

  Duration& operator/=(float rhs) {
    *this = *this / rhs;
    return *this;
  }

  bool operator==(const Duration& rhs) const {
    return ns_count_ == rhs.ns_count_;
  }

 private:
  explicit Duration(internal::InfinityType inf_type) : inf_type_(inf_type) {}

  internal::InfinityType inf_type_ = internal::InfinityType::kNone;
  long long ns_count_ = 0;
};

YOGI_WEAK_SYMBOL const Duration
    Duration::kInfinity(internal::InfinityType::kPositive);
YOGI_WEAK_SYMBOL const Duration
    Duration::kNegativeInfinity(internal::InfinityType::kNegative);

}  // namespace yogi
