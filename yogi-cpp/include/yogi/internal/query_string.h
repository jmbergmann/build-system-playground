#pragma once

#include "../errors.h"

#include <string>

namespace yogi {
namespace internal {

template <typename Fn>
inline std::string QueryString(Fn fn) {
  std::vector<char> v;
  int size = 64;

  int res;
  do {
    size *= 2;
    v = std::vector<char>{};
    v.resize(size);

    res = fn(v.data(), size);
  } while (res == static_cast<int>(ErrorCode::kBufferTooSmall));

  return std::string(v.data());
}

}  // namespace internal
}  // namespace yogi
