#pragma once

#include <string>

namespace yogi {
namespace internal {

inline const char* ToCoreString(const char* s) { return s; }

inline const char* ToCoreString(const std::string& s) {
  return s.c_str();
}

}  // namespace internal
}  // namespace yogi
