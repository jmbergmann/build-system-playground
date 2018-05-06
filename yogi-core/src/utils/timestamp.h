#pragma once

#include "../config.h"

#include <chrono>

namespace utils {

class Timestamp {
 public:
  static Timestamp Now();

  Timestamp() {}

  long long NanosecondsSinceEpoch() const {
    return static_cast<long long>(time_.time_since_epoch().count());
  }

  int NanosecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch() / 1) % 1000);
  }

  int MicrosecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch() / 1000) % 1000);
  }

  int MillisecondsFraction() const {
    return static_cast<int>((NanosecondsSinceEpoch() / 1000000) % 1000);
  }

  std::string ToFormattedString(std::string fmt) const;
  std::string ToJavaScriptString() const;

 private:
  typedef std::chrono::time_point<std::chrono::system_clock,
                                  std::chrono::nanoseconds>
      TimePoint;

  Timestamp(const TimePoint& time) : time_(time) {}

  TimePoint time_;
};

}  // namespace utils
