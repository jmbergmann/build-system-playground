#pragma once

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

namespace yogi {
namespace internal {

inline long long DurationToCoreDuration(std::chrono::nanoseconds duration) {
  return duration == (duration.max)() ? -1 : duration.count();
}

inline std::chrono::nanoseconds CoreDurationToDuration(long long duration) {
  return duration == -1 ? (std::chrono::nanoseconds::max)()
                        : std::chrono::nanoseconds(duration);
}

inline const char* StringToCoreString(const char* s) { return s; }

inline const char* StringToCoreString(const std::string& s) {
  return s.c_str();
}

}  // namespace internal
}  // namespace yogi
