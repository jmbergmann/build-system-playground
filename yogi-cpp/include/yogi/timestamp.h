#pragma once

#include "internal/conversion.h"
#include "internal/error_code_helpers.h"
#include "internal/library.h"
#include "duration.h"

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
  /// Creates a timestamp from a duration since the epoch 01/01/1970 UTC.
  ///
  /// \returns Timestamp instance.
  static Timestamp FromDurationSinceEpoch(const Duration& dur_since_epoch) {
    if (dur_since_epoch.IsInfinite() || dur_since_epoch < Duration()) {
      throw ArithmeticException(
          "Invalid duration range for use as a timestamp");
    }

    Timestamp t;
    t.dur_since_epoch_ = dur_since_epoch;
    return t;
  }

  /// Creates a timestamp from the current time.
  ///
  /// \returns Timestamp corresponding to the current time.
  static Timestamp Now() {
    long long timestamp;
    int res = internal::YOGI_GetCurrentTime(&timestamp);
    internal::CheckErrorCode(res);

    return FromDurationSinceEpoch(Duration::FromNanoseconds(timestamp));
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
  /// By default, the string \p str will be parsed in the format
  /// "2018-04-23T18:25:43.511Z".
  ///
  /// \tparam StrString Type of the \p str string.
  /// \tparam FmtString Type of the \p timefmt string.
  ///
  /// \param str The string to parse.
  /// \param timefmt Format of the time string.
  ///
  /// \returns The parsed timestamp.
  template <typename StrString, typename FmtString = char*>
  static Timestamp Parse(const StrString& str,
                         const FmtString& timefmt = nullptr) {
    long long timestamp;
    int res =
        internal::YOGI_ParseTime(&timestamp, internal::StringToCoreString(str),
                                 internal::StringToCoreString(timefmt));
    internal::CheckErrorCode(res);
    return FromDurationSinceEpoch(Duration::FromNanoseconds(timestamp));
  }

  /// Constructs the timestamp using the epoch 01/01/1970 UTC.
  Timestamp() {}

  /// Returns the duration since 01/01/1970 UTC.
  ///
  /// \returns Duration since 01/01/1970 UTC.
  Duration DurationSinceEpoch() const { return dur_since_epoch_; }

  /// Returns the nanoseconds fraction of the timestamp.
  ///
  /// \returns Nanoseconds fraction of the timestamp (0-999).
  int Nanoseconds() const { return dur_since_epoch_.Nanoseconds(); }

  /// Returns the microseconds fraction of the timestamp.
  ///
  /// \returns Microseconds fraction of the timestamp (0-999).
  int Microseconds() const { return dur_since_epoch_.Microseconds(); }

  /// Returns the Milliseconds fraction of the timestamp.
  ///
  /// \returns Milliseconds fraction of the timestamp (0-999).
  int Milliseconds() const { return dur_since_epoch_.Milliseconds(); }

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
  /// By default, the timestamp will be formatted as an ISO-8601 string with
  /// up to millisecond resolution, e.g. "2018-04-23T18:25:43.511Z".
  ///
  /// \tparam String Type of the \p timefmt string.
  ///
  /// \param timefmt Format of the time string.
  ///
  /// \returns The formatted time string.
  template <typename String = char*>
  std::string Format(const String& timefmt = nullptr) const {
    char str[128];
    int res = internal::YOGI_FormatTime(dur_since_epoch_.TotalNanoseconds(),
                                        str, sizeof(str),
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

  Timestamp operator+(const Duration& dur) const {
    return FromDurationSinceEpoch(dur_since_epoch_ + dur);
  }

  Timestamp operator-(const Duration& dur) const {
    return FromDurationSinceEpoch(dur_since_epoch_ - dur);
  }

  Duration operator-(const Timestamp& rhs) const {
    return dur_since_epoch_ - rhs.dur_since_epoch_;
  }

  Timestamp& operator+=(const Duration& dur) {
    *this = *this + dur;
    return *this;
  }

  Timestamp& operator-=(const Duration& dur) {
    *this = *this - dur;
    return *this;
  }

  bool operator==(const Timestamp& rhs) const {
    return dur_since_epoch_ == rhs.dur_since_epoch_;
  }

  bool operator!=(const Timestamp& rhs) const {
    return dur_since_epoch_ != rhs.dur_since_epoch_;
  }

  bool operator<(const Timestamp& rhs) const {
    return dur_since_epoch_ < rhs.dur_since_epoch_;
  }

  bool operator>(const Timestamp& rhs) const {
    return dur_since_epoch_ > rhs.dur_since_epoch_;
  }

  bool operator<=(const Timestamp& rhs) const {
    return dur_since_epoch_ <= rhs.dur_since_epoch_;
  }

  bool operator>=(const Timestamp& rhs) const {
    return dur_since_epoch_ >= rhs.dur_since_epoch_;
  }

 private:
  Duration dur_since_epoch_;
};

}  // namespace yogi
