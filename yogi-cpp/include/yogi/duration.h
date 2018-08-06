#pragma once

#include "internal/library.h"
#include "internal/conversion.h"
#include "internal/error_code_helpers.h"

#include <chrono>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <type_traits>

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_FormatDuration,
                   (long long dur, int neg, char* str, int strsize,
                    const char* durfmt, const char* infstr))

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
    throw ArithmeticException("Duration value overflow.");
  } else if (a < 0 && b < (std::numeric_limits<long long>::min)() - a) {
    throw ArithmeticException("Duration value underflow.");
  }

  return a + b;
}

template <typename T>
inline long long MultiplySafely(long long val, T multiplicator) {
  if (IsNan(multiplicator)) {
    throw ArithmeticException("Trying to multiply duration value and NaN.");
  }

  if (multiplicator == T{}) {
    return 0;
  }

  long long max_val = static_cast<long long>(
      (std::numeric_limits<long long>::max)() / multiplicator);
  if (std::abs(val) > max_val) {
    throw ArithmeticException("Duration value overflow.");
  }

  return static_cast<long long>(val * multiplicator);
}

template <typename T>
void CheckDivisor(T divisor) {
  if (IsNan(divisor)) {
    throw ArithmeticException("Trying to divide duration value by NaN.");
  }

  if (divisor == T{}) {
    throw ArithmeticException("Trying to divide duration value by zero.");
  }
}

template <typename T>
inline long long DivideSafely(long long val, T divisor) {
  CheckDivisor(divisor);

  if (!IsFinite(divisor)) {
    return 0;
  }

  if (std::abs(divisor) < static_cast<T>(1)) {
    long long max_val = static_cast<long long>(
        (std::numeric_limits<long long>::max)() * divisor);
    if (std::abs(val) > max_val) {
      throw ArithmeticException("Duration value overflow.");
    }
  }

  return static_cast<long long>(val / divisor);
}

}  // namespace internal

/// Represents a time duration.
///
/// The resolution of a time duration is in nanoseconds.
class Duration {
 public:
  /// Integral type used to represent the number of nanoseconds
  typedef long long value_type;

  /// Zero duration
  static const Duration kZero;

  /// Infinite duration
  static const Duration kInfinity;

  /// Negative infinite duration
  static const Duration kNegativeInfinity;

  /// Construct a duration from a number of nanoseconds
  ///
  /// \param ns Number of nanoseconds
  ///
  /// \returns Duration instance
  static Duration FromNanoseconds(value_type ns) {
    return Duration(std::chrono::nanoseconds(ns));
  }

  /// Construct a duration from a number of microseconds
  ///
  /// \param us Number of microseconds
  ///
  /// \returns Duration instance
  static Duration FromMicroseconds(value_type us) {
    return FromNanoseconds(internal::MultiplySafely(us, 1e3));
  }

  /// Construct a duration from a number of milliseconds
  ///
  /// \param ms Number of milliseconds
  ///
  /// \returns Duration instance
  static Duration FromMilliseconds(value_type ms) {
    return FromNanoseconds(internal::MultiplySafely(ms, 1e6));
  }

  /// Construct a duration from a number of seconds
  ///
  /// \param seconds Number of seconds
  ///
  /// \returns Duration instance
  static Duration FromSeconds(value_type seconds) {
    return FromNanoseconds(internal::MultiplySafely(seconds, 1e9));
  }

  /// Construct a duration from a number of minutes
  ///
  /// \param minutes Number of minutes
  ///
  /// \returns Duration instance
  static Duration FromMinutes(value_type minutes) {
    return FromNanoseconds(internal::MultiplySafely(minutes, 60 * 1e9));
  }

  /// Construct a duration from a number of hours
  ///
  /// \param hours Number of hours
  ///
  /// \returns Duration instance
  static Duration FromHours(value_type hours) {
    return FromNanoseconds(internal::MultiplySafely(hours, 60 * 60 * 1e9));
  }

  /// Construct a duration from a number of days
  ///
  /// \param days Number of days
  ///
  /// \returns Duration instance
  static Duration FromDays(value_type days) {
    return FromNanoseconds(internal::MultiplySafely(days, 24 * 60 * 60 * 1e9));
  }

  /// Constructs a zero nanosecond duration
  Duration() : Duration(internal::InfinityType::kNone) {}

  /// Constructs a duration from a duration value from the standard library
  ///
  /// \tparam Rep    Arithmetic type representing the number of ticks
  /// \tparam Period Ratio type representing the tick period
  ///
  /// \param dur Duration
  template <typename Rep, typename Period>
  Duration(const std::chrono::duration<Rep, Period>& dur)
      : Duration(internal::InfinityType::kNone) {
    ns_count_ =
        std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
  }

  /// Nanoseconds fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Nanoseconds fraction (0-999)
  int Nanoseconds() const {
    return static_cast<int>(TotalNanoseconds() % 1000);
  }

  /// Microseconds fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Microseconds fraction (0-999)
  int Microseconds() const {
    return static_cast<int>(TotalMicroseconds() % 1000);
  }

  /// Milliseconds fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Milliseconds fraction (0-999)
  int Milliseconds() const {
    return static_cast<int>(TotalMilliseconds() % 1000);
  }

  /// Seconds fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Seconds fraction (0-59)
  int Seconds() const { return static_cast<int>(TotalSeconds() % 60); }

  /// Minutes fraction of the duration
  ///
  /// \returns Minutes fraction (0-59)
  int Minutes() const { return static_cast<int>(TotalMinutes() % 60); }

  /// Hours fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Hours fraction (0-23)
  int Hours() const { return static_cast<int>(TotalHours() % 24); }

  /// Days fraction of the duration
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Days fraction
  int Days() const { return static_cast<int>(TotalDays()); }

  /// Total number of nanoseconds
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Total number of nanoseconds
  value_type TotalNanoseconds() const { return ns_count_; }

  /// Total number of microseconds
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Total number of microseconds
  value_type TotalMicroseconds() const { return ns_count_ / 1'000; }

  /// Total number of milliseconds
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Total number of milliseconds
  value_type TotalMilliseconds() const { return ns_count_ / 1'000'000; }

  /// Total number of seconds
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Total number of seconds
  value_type TotalSeconds() const { return ns_count_ / 1'000'000'000; }

  /// Total number of minutes
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Total number of minutes
  value_type TotalMinutes() const { return TotalSeconds() / 60; }

  /// Total number of hours
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Total number of hours
  value_type TotalHours() const { return TotalMinutes() / 60; }

  /// Total number of days
  ///
  /// If the duration is infinite, this function will return an undefined value.
  ///
  /// \returns Total number of days
  value_type TotalDays() const { return TotalHours() / 24; }

  /// Returns the negated duration, i.e. multiplied by -1
  ///
  /// \returns Negated duration
  Duration Negated() const {
    switch (inf_type_) {
      case internal::InfinityType::kPositive:
        return Duration(internal::InfinityType::kNegative);

      case internal::InfinityType::kNegative:
        return Duration(internal::InfinityType::kPositive);

      default:
        return FromNanoseconds(-ns_count_);
    }
  }

  /// Converts the duration to a standard library duration
  ///
  /// In case that the duration is infinite, the respective min or max value
  /// of the requested type will be returned.
  ///
  /// \tparam T Standard library duration type
  ///
  /// \returns Duration as a standard library duration
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

  /// Checks if the duration is infinite or not
  ///
  /// \returns True if the duration is infinite
  bool IsInfinite() const { return inf_type_ != internal::kNone; }

  /// Converts the duration to a string.
  ///
  /// The \p dur_fmt parameter describes the format of the conversion. The
  /// following placeholders are supported:
  ///  - *%+*: Plus sign if duration is positive; minus sign if it is negative
  ///  - *%-*: Minus sign (only) if duration is negative
  ///  - *%d*: Total number of days
  ///  - *%D*: Total number of days if days > 0
  ///  - *%H*: Fractional hours (range 00 to 23)
  ///  - *%M*: Fractional minutes (range 00 to 59)
  ///  - *%S*: Fractional seconds (range 00 to 59)
  ///  - *%T*: Equivalent to %H:%M:%S
  ///  - *%3*: Fractional milliseconds (range 000 to 999)
  ///  - *%6*: Fractional microseconds (range 000 to 999)
  ///  - *%9*: Fractional nanoseconds (range 000 to 999)
  ///
  /// The \p inf_fmt parameter describes the format to use for infinite
  /// durations. The following placeholders are supported:
  ///  - *%+*: Plus sign if duration is positive; minus sign if it is negative
  ///  - *%-*: Minus sign (only) if duration is negative
  ///
  /// By default, the duration will be formatted in the format
  /// "-23d 04:19:33.123456789". If the duration is infinite, then the string
  /// constructed using \p inf_fmt will be returned ("inf" and "-inf"
  /// respectively by default).
  ///
  /// \tparam DurFmtString Type of the \p dur_fmt parameter
  /// \tparam InfFmtString Type of the \p inf_fmt parameter
  ///
  /// \param[in] dur_fmt Format of the duration string
  /// \param[in] inf_fmt String to use for infinity
  ///
  /// \returns The formatted duration string
  template <typename DurFmtString = char*, typename InfFmtString = char*>
  std::string Format(const DurFmtString& dur_fmt = nullptr,
                     const InfFmtString& inf_fmt = nullptr) const {
    char str[128];
    int res = internal::YOGI_FormatDuration(
        IsInfinite() ? -1 : ns_count_, ns_count_ < 0 ? 1 : 0, str, sizeof(str),
        internal::StringToCoreString(dur_fmt),
        internal::StringToCoreString(inf_fmt));
    internal::CheckErrorCode(res);
    return str;
  }

  /// Converts the duration to a string
  ///
  /// The format of the string will be "-23d 04:19:33.123456789".
  ///
  /// \returns Duration as a string
  std::string ToString() const { return Format(); }

  Duration operator+(const Duration& rhs) const {
    if (inf_type_ == internal::kNone && rhs.inf_type_ == internal::kNone) {
      return FromNanoseconds(internal::AddSafely(ns_count_, rhs.ns_count_));
    }

    auto inf_type_sum = inf_type_ + rhs.inf_type_;
    if (inf_type_sum == internal::kNone) {
      throw ArithmeticException(
          "Trying to add positive and negative infinite duration values.");
    }

    return inf_type_sum > 0 ? kInfinity : kNegativeInfinity;
  }

  Duration operator-(const Duration& rhs) const {
    return *this + rhs.Negated();
  }

  template <typename T>
  Duration operator*(T rhs) const {
    static_assert(std::is_arithmetic<T>::value,
                  "T must be an arithmetic type.");

    if (inf_type_ != internal::kNone && rhs == T{}) {
      throw ArithmeticException(
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

    internal::CheckDivisor(rhs);
    auto rhs_inf_type = rhs > T{} ? internal::kPositive : internal::kNegative;
    return inf_type_ == rhs_inf_type ? kInfinity : kNegativeInfinity;
  }

  Duration& operator+=(const Duration& rhs) {
    *this = *this + rhs;
    return *this;
  }

  Duration& operator-=(const Duration& rhs) {
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
    if (inf_type_ == rhs.inf_type_ &&
        inf_type_ != internal::InfinityType::kNone) {
      return true;
    }

    return ns_count_ == rhs.ns_count_;
  }

  bool operator!=(const Duration& rhs) const { return !(*this == rhs); }

  bool operator<(const Duration& rhs) const {
    if (inf_type_ == rhs.inf_type_ &&
        inf_type_ == internal::InfinityType::kNone) {
      return ns_count_ < rhs.ns_count_;
    }

    return inf_type_ < rhs.inf_type_ ? true : false;
  }

  bool operator>(const Duration& rhs) const {
    if (inf_type_ == rhs.inf_type_ &&
        inf_type_ == internal::InfinityType::kNone) {
      return ns_count_ > rhs.ns_count_;
    }

    return inf_type_ > rhs.inf_type_ ? true : false;
  }

  bool operator<=(const Duration& rhs) const { return !(*this > rhs); }

  bool operator>=(const Duration& rhs) const { return !(*this < rhs); }

 private:
  explicit Duration(internal::InfinityType inf_type) : inf_type_(inf_type) {
    switch (inf_type) {
      case internal::InfinityType::kNone:
        ns_count_ = 0;
        break;

      case internal::InfinityType::kPositive:
        ns_count_ = (std::numeric_limits<value_type>::max)();
        break;

      case internal::InfinityType::kNegative:
        ns_count_ = (std::numeric_limits<value_type>::min)();
        break;
    }
  }

  internal::InfinityType inf_type_;
  value_type ns_count_;
};

YOGI_WEAK_SYMBOL const Duration Duration::kZero;
YOGI_WEAK_SYMBOL const Duration
    Duration::kInfinity(internal::InfinityType::kPositive);
YOGI_WEAK_SYMBOL const Duration
    Duration::kNegativeInfinity(internal::InfinityType::kNegative);

}  // namespace yogi
