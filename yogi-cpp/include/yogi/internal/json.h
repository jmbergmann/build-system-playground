#pragma once

#include <chrono>
#include <string>

namespace yogi {
namespace internal {

template <typename T>
inline T ExtractFromJson(const std::string& json, const char* name) {
  return {};
}

}  // namespace internal
}  // namespace yogi
