#pragma once

#include <chrono>

namespace yogi {
namespace internal {

inline long long DurationToCoreDuration(std::chrono::nanoseconds duration) {
  return duration == (duration.max)() ? -1 : duration.count();
}

}  // namespace internal
}  // namespace yogi
