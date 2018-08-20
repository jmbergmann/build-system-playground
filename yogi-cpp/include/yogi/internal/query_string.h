#pragma once

#include "../errors.h"
#include "error_code_helpers.h"

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
    v.resize(static_cast<std::size_t>(size));

    res = fn(v.data(), size);
  } while (res == static_cast<int>(ErrorCode::kBufferTooSmall));

  internal::CheckErrorCode(res);

  return std::string(v.data());
}

}  // namespace internal
}  // namespace yogi
