#include "macros.h"
#include "helpers.h"
#include "../api/constants.h"
#include "../utils/timestamp.h"

YOGI_API int YOGI_GetCurrentTime(long long* timestamp) {
  CHECK_PARAM(timestamp != nullptr);

  try {
    auto now = utils::Timestamp::Now();
    *timestamp = now.NanosecondsSinceEpoch().count();
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_FormatTime(long long timestamp, char* str, int strsize,
                             const char* timefmt) {
  CHECK_PARAM(str != nullptr);
  CHECK_PARAM(strsize > 0);

  try {
    auto t = utils::Timestamp(std::chrono::nanoseconds(timestamp));
    auto s = t.ToFormattedString(timefmt ? timefmt : api::kDefaultTimeFormat);
    if (!CopyStringToUserBuffer(s, str, strsize)) {
      return YOGI_ERR_BUFFER_TOO_SMALL;
    }
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_ParseTime(long long* timestamp, const char* str,
                            const char* timefmt) {
  CHECK_PARAM(timestamp != nullptr);
  CHECK_PARAM(str != nullptr);

  try {
    auto t = utils::Timestamp::Parse(
        str, timefmt ? timefmt : api::kDefaultTimeFormat);
    *timestamp = t.NanosecondsSinceEpoch().count();
  }
  CATCH_AND_RETURN;
}
