#pragma once

#include "internal/conversion.h"
#include "internal/error_code_helpers.h"
#include "internal/library.h"

#include <chrono>
#include <sstream>
#include <iomanip>

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_GetCurrentTime, (long long* timestamp))

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

  /// Constructs the timestamp using the epoch 01/01/1970 UTC.
  Timestamp() : time_(0) {}

  /// Construct the timestamp from a duration since 01/01/1970 UTC.
  explicit Timestamp(const std::chrono::nanoseconds& ns_since_epoch)
      : time_(ns_since_epoch) {}

  /// Returns the number of nanoseconds since 01/01/1970 UTC.
  ///
  /// \returns Number of nanoseconds since 01/01/1970 UTC.
  std::chrono::nanoseconds NanosecondsSinceEpoch() const { return time_; }

  /// Returns the nanoseconds fraction of the timestamp.
  ///
  /// \returns Nanoseconds fraction of the timestamp (0-999).
  int NanosecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch().count() / 1) % 1000);
  }

  /// Returns the microseconds fraction of the timestamp.
  ///
  /// \returns Microseconds fraction of the timestamp (0-999).
  int MicrosecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch().count() / 1000) % 1000);
  }

  /// Returns the Milliseconds fraction of the timestamp.
  ///
  /// \returns Milliseconds fraction of the timestamp (0-999).
  int MillisecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch().count() / 1000000) % 1000);
  }

  /// Returns the time in ISO-8601 format up to milliseonds.
  ///
  /// Example: "2018-04-23T18:25:43.511Z".
  ///
  /// \returns The time in ISO-8601 format up to milliseonds.
  std::string ToString() const {
    auto time = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::time_point() +
        std::chrono::duration_cast<std::chrono::system_clock::duration>(time_));

    std::tm tm;
#ifdef _WIN32
    gmtime_s(&tm, &time);
#else
    gmtime_r(&time, &tm);
#endif

    std::stringstream ss;
    ss << std::put_time(&tm, "%FT%T");
    ss << '.';
    ss << std::setw(3) << std::setfill('0') << MillisecondsFraction();
    ss << 'Z';
    return ss.str();
  }

  Timestamp operator+ (const std::chrono::nanoseconds& ns) const {
    return Timestamp(NanosecondsSinceEpoch() + ns);
  }

  Timestamp operator- (const std::chrono::nanoseconds& ns) const {
    return Timestamp(NanosecondsSinceEpoch() - ns);
  }

  Timestamp& operator+= (const std::chrono::nanoseconds& ns) {
    *this = *this + ns;
    return *this;
  }

  Timestamp& operator-= (const std::chrono::nanoseconds& ns) {
    *this = *this - ns;
    return *this;
  }

  bool operator== (const Timestamp& rhs) const {
    return NanosecondsSinceEpoch() == rhs.NanosecondsSinceEpoch();
  }

  bool operator!= (const Timestamp& rhs) const {
    return NanosecondsSinceEpoch() != rhs.NanosecondsSinceEpoch();
  }

  bool operator< (const Timestamp& rhs) const {
    return NanosecondsSinceEpoch() < rhs.NanosecondsSinceEpoch();
  }

  bool operator> (const Timestamp& rhs) const {
    return NanosecondsSinceEpoch() > rhs.NanosecondsSinceEpoch();
  }

  bool operator<= (const Timestamp& rhs) const {
    return !(*this > rhs);
  }

  bool operator>= (const Timestamp& rhs) const {
    return !(*this < rhs);
  }

 private:
  std::chrono::nanoseconds time_;
};

}  // namespace yogi
