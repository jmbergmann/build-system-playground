#pragma once

#include "internal/library.h"


namespace yogi {

YOGI_DEFINE_API_FN(const char*, YOGI_GetVersion, ())

inline const std::string& GetVersion() {
  static std::string s = internal::YOGI_GetVersion();
  return s;
}

}  // namespace yogi
