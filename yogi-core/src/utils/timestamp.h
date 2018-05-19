#pragma once

#include "../config.h"

#include <chrono>

namespace utils {

class Timestamp {
 public:
  static Timestamp Now();

  Timestamp() {}
  Timestamp(const std::chrono::nanoseconds& nanoseconds_since_epoch)
      : time_(TimePoint() + nanoseconds_since_epoch){};

  std::chrono::nanoseconds NanosecondsSinceEpoch() const {
    return time_.time_since_epoch();
  }

  int NanosecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch().count() / 1) % 1000);
  }

  int MicrosecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch().count() / 1000) % 1000);
  }

  int MillisecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch().count() / 1000000) % 1000);
  }

  std::string ToFormattedString(std::string fmt) const;
  std::string ToJavaScriptString() const;

  Timestamp operator+ (const std::chrono::nanoseconds& ns) const {
    return Timestamp(NanosecondsSinceEpoch() + ns);
  }

  Timestamp operator- (const std::chrono::nanoseconds& ns) const {
    return Timestamp(NanosecondsSinceEpoch() - ns);
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

 private:
  typedef std::chrono::time_point<std::chrono::system_clock,
                                  std::chrono::nanoseconds>
      TimePoint;

  Timestamp(const TimePoint& time) : time_(time) {}

  TimePoint time_;
};

}  // namespace utils
