#pragma once

#include <yogi_core.h>

#include <string>

namespace yogi {

inline const std::string& GetVersion() {
  static std::string s = YOGI_GetVersion();
  return s;
}

} // namespace yogi
