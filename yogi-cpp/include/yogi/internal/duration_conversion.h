#pragma once

#include "../errors.h"
#include "../duration.h"

namespace yogi {
namespace internal {

inline long long ToCoreDuration(const Duration& duration) {
  if (duration < Duration::kZero) {
    throw FailureException(ErrorCode::kInvalidParam);
  }

  return duration.IsFinite() ? duration.NanosecondsCount() : -1;
}

}  // namespace internal
}  // namespace yogi
