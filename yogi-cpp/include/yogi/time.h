#ifndef YOGI_TIME_H
#define YOGI_TIME_H

#include "timestamp.h"

namespace yogi {

/// Get the current time.
///
/// \returns The current time since 01/01/1970 UTC.
inline Timestamp GetCurrentTime() { return Timestamp::Now(); }

}  // namespace yogi

#endif  // YOGI_TIME_H
