#pragma once

#include <chrono>

namespace yogi {
namespace internal {

inline long long DurationToCoreDuration(std::chrono::nanoseconds duration) {
  return duration == (duration.max)() ? -1 : duration.count();
}

inline std::chrono::nanoseconds CoreDurationToDuration(long long duration) {
  return duration == -1 ? (std::chrono::nanoseconds::max)()
                        : std::chrono::nanoseconds(duration);
}

}  // namespace internal
}  // namespace yogi
