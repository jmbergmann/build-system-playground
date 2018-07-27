#pragma once

#include "timestamp.h"

namespace yogi {

/// Get the current time.
///
/// \returns The current time since 01/01/1970 UTC.
inline Timestamp GetCurrentTime() {
  return Timestamp::Now();
}

}  // namespace yogi
