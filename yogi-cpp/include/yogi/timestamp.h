#pragma once

#include "internal/conversion.h"
#include "internal/error_code_helpers.h"
#include "internal/library.h"

#include <chrono>
#include <sstream>
#include <iomanip>

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_GetCurrentTime, (long long* timestamp))
YOGI_DEFINE_API_FN(int, YOGI_FormatTime,
                   (long long timestamp, char* str, int strsize,
                    const char* timefmt))
YOGI_DEFINE_API_FN(int, YOGI_ParseTime,
                   (long long* timestamp, const char* str, const char* timefmt))

/// Represents a UTC timestamp.
///
/// Timestamps are expressed in nanoseconds since 01/01/1970 UTC.
class Timestamp {
 public:
  /// Creates a timestamp from the current time.
  ///
  /// \returns Timestamp corresponding to the current time.
  static Timestamp Now() {
    long long timestamp;
    int res = internal::YOGI_GetCurrentTime(&timestamp);
    internal::CheckErrorCode(res);

    return Timestamp(std::chrono::nanoseconds(timestamp));
  }

  /// Converts a string into a timestamp.
  ///
  /// The string \p str will be parsed in the format
  /// "2018-04-23T18:25:43.511Z".
  ///
  /// \tparam StrString Type of the \p str string.
  ///
  /// \param str The string to parse.
  ///
  /// \returns The parsed timestamp.
  template <typename StrString>
  static Timestamp Parse(const StrString& str) {
    long long timestamp;
    int res = internal::YOGI_ParseTime(
        &timestamp, internal::StringToCoreString(str), nullptr);
    internal::CheckErrorCode(res);
    return Timestamp(std::chrono::nanoseconds(timestamp));
  }

  /// Converts a string into a timestamp.
  ///
  /// The \p timefmt parameter describes the format of the conversion. The
  /// following placeholders are supported:
  ///  - *%Y*: Four digit year
  ///  - *%m*: Month name as a decimal 01 to 12
  ///  - *%d*: Day of the month as decimal 01 to 31
  ///  - *%F*: Equivalent to %Y-%m-%d (the ISO 8601 date format)
  ///  - *%H*: The hour as a decimal number using a 24-hour clock (00 to 23)
  ///  - *%M*: The minute as a decimal 00 to 59
  ///  - *%S*: Seconds as a decimal 00 to 59
  ///  - *%T*: Equivalent to %H:%M:%S (the ISO 8601 time format)
  ///  - *%3*: Milliseconds as decimal number 000 to 999
  ///  - *%6*: Microseconds as decimal number 000 to 999
  ///  - *%9*: Nanoseconds as decimal number 000 to 999
  ///
  /// \tparam StrString Type of the \p str string.
  /// \tparam FmtString Type of the \p timefmt string.
  ///
  /// \param str The string to parse.
  /// \param timefmt Format of the time string.
  ///
  /// \returns The parsed timestamp.
  template <typename StrString, typename FmtString>
  static Timestamp Parse(const StrString& str, const FmtString& timefmt) {
    long long timestamp;
    int res =
        internal::YOGI_ParseTime(&timestamp, internal::StringToCoreString(str),
                                 internal::StringToCoreString(timefmt));
    internal::CheckErrorCode(res);
    return Timestamp(std::chrono::nanoseconds(timestamp));
  }

  /// Constructs the timestamp using the epoch 01/01/1970 UTC.
  Timestamp() : ns_since_epoch_(0) {}

  /// Construct the timestamp from a duration since 01/01/1970 UTC.
  explicit Timestamp(const std::chrono::nanoseconds& ns_since_epoch)
      : ns_since_epoch_(ns_since_epoch.count()) {}

  /// Returns the number of nanoseconds since 01/01/1970 UTC.
  ///
  /// \returns Number of nanoseconds since 01/01/1970 UTC.
  std::chrono::nanoseconds DurationSinceEpoch() const {
    return std::chrono::nanoseconds(ns_since_epoch_);
  }

  /// Returns the nanoseconds fraction of the timestamp.
  ///
  /// \returns Nanoseconds fraction of the timestamp (0-999).
  int Nanosecond() const {
    return static_cast<int>((ns_since_epoch_ / 1) % 1000);
  }

  /// Returns the microseconds fraction of the timestamp.
  ///
  /// \returns Microseconds fraction of the timestamp (0-999).
  int Microsecond() const {
    return static_cast<int>((ns_since_epoch_ / 1000) % 1000);
  }

  /// Returns the Milliseconds fraction of the timestamp.
  ///
  /// \returns Milliseconds fraction of the timestamp (0-999).
  int Millisecond() const {
    return static_cast<int>((ns_since_epoch_ / 1000000) % 1000);
  }

  /// Converts the timestamp to an ISO-8601 string up to milliseconds.
  ///
  /// Example: "2018-04-23T18:25:43.511Z".
  ///
  /// \returns The time in ISO-8601 format up to milliseonds.
  ///
  /// \returns The formatted time string.
  std::string Format() const {
    char str[32];
    int res =
        internal::YOGI_FormatTime(ns_since_epoch_, str, sizeof(str), nullptr);
    internal::CheckErrorCode(res);
    return str;
  }

  /// Converts the timestamp to a string.
  ///
  /// The \p timefmt parameter describes the format of the conversion. The
  /// following placeholders are supported:
  ///  - *%Y*: Four digit year
  ///  - *%m*: Month name as a decimal 01 to 12
  ///  - *%d*: Day of the month as decimal 01 to 31
  ///  - *%F*: Equivalent to %Y-%m-%d (the ISO 8601 date format)
  ///  - *%H*: The hour as a decimal number using a 24-hour clock (00 to 23)
  ///  - *%M*: The minute as a decimal 00 to 59
  ///  - *%S*: Seconds as a decimal 00 to 59
  ///  - *%T*: Equivalent to %H:%M:%S (the ISO 8601 time format)
  ///  - *%3*: Milliseconds as decimal number 000 to 999
  ///  - *%6*: Microseconds as decimal number 000 to 999
  ///  - *%9*: Nanoseconds as decimal number 000 to 999
  ///
  /// \tparam String Type of the \p timefmt string.
  ///
  /// \param timefmt Format of the time string.
  ///
  /// \returns The formatted time string.
  template <typename String>
  std::string Format(const String& timefmt) const {
    char str[128];
    int res = internal::YOGI_FormatTime(ns_since_epoch_, str, sizeof(str),
                                        internal::StringToCoreString(timefmt));
    internal::CheckErrorCode(res);
    return str;
  }

  /// Returns the time in ISO-8601 format up to milliseonds.
  ///
  /// Example: "2018-04-23T18:25:43.511Z".
  ///
  /// \returns The time in ISO-8601 format up to milliseonds.
  std::string ToString() const { return Format(); }

  Timestamp operator+(const std::chrono::nanoseconds& ns) const {
    return Timestamp(DurationSinceEpoch() + ns);
  }

  Timestamp operator-(const std::chrono::nanoseconds& ns) const {
    return Timestamp(DurationSinceEpoch() - ns);
  }

  std::chrono::nanoseconds operator-(const Timestamp& rhs) const {
    return DurationSinceEpoch() - rhs.DurationSinceEpoch();
  }

  Timestamp& operator+=(const std::chrono::nanoseconds& ns) {
    *this = *this + ns;
    return *this;
  }

  Timestamp& operator-=(const std::chrono::nanoseconds& ns) {
    *this = *this - ns;
    return *this;
  }

  bool operator==(const Timestamp& rhs) const {
    return DurationSinceEpoch() == rhs.DurationSinceEpoch();
  }

  bool operator!=(const Timestamp& rhs) const {
    return DurationSinceEpoch() != rhs.DurationSinceEpoch();
  }

  bool operator<(const Timestamp& rhs) const {
    return DurationSinceEpoch() < rhs.DurationSinceEpoch();
  }

  bool operator>(const Timestamp& rhs) const {
    return DurationSinceEpoch() > rhs.DurationSinceEpoch();
  }

  bool operator<=(const Timestamp& rhs) const { return !(*this > rhs); }

  bool operator>=(const Timestamp& rhs) const { return !(*this < rhs); }

 private:
  std::chrono::nanoseconds::rep ns_since_epoch_;
};

}  // namespace yogi
