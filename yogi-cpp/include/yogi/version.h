#pragma once

#include "internal/library.h"


namespace yogi {

inline const std::string& GetVersion() {
  static std::string s = Library::GetVersion();
  return s;
}

}  // namespace yogi
