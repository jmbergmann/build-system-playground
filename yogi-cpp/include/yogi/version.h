#pragma once

#include "internal/library.h"


namespace yogi {

YOGI_DEFINE_API_FN(const char*, YOGI_GetVersion, ())

/// Returns the version string of the loaded Yogi Core library.
///
/// \returns Version string of the loaded Yogi Core library.
inline const std::string& GetVersion() {
  static std::string s = internal::YOGI_GetVersion();
  return s;
}

}  // namespace yogi
