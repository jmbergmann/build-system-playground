#pragma once

#include "internal/conversion.h"
#include "internal/error_code_helpers.h"
#include "internal/library.h"

#include <chrono>


namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_GetCurrentTime, (long long* timestamp))

/// Get the current time.
///
/// \returns The current time since 01/01/1970 UTC.
inline std::chrono::system_clock::time_point GetCurrentTime() {
  long long timestamp;
  int res = internal::YOGI_GetCurrentTime(&timestamp);
  internal::CheckErrorCode(res);

  return internal::CoreTimestampToTimestamp(timestamp);
}

}  // namespace yogi
