#pragma once

#include "internal/library.h"
#include "internal/error_code_helpers.h"

#include <chrono>


namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_GetCurrentTime, (long long* timestamp))

/***************************************************************************//**
 * Get the current time.
 *
 * \returns The current time since 01/01/1970 UTC.
 ******************************************************************************/
inline std::chrono::system_clock::time_point GetCurrentTime() {
  long long timestamp;
  int res = internal::YOGI_GetCurrentTime(&timestamp);
  internal::CheckErrorCode(res);

  std::chrono::system_clock::time_point tp;
  tp += std::chrono::duration_cast<std::chrono::system_clock::duration>(
    std::chrono::nanoseconds(timestamp));

  return tp;
}

}  // namespace yogi
