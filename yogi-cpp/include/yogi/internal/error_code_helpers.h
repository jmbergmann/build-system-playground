#pragma once

#include "../errors.h"

#include <array>

namespace yogi {
namespace internal {

inline void CheckErrorCode(int res) {
  if (res < 0) {
    throw FailureException(static_cast<ErrorCode>(res));
  }
}

template <typename Fn>
inline void CheckDescriptiveErrorCode(Fn fn) {
  std::array<char, 256> description;
  int res = fn(description.data(), description.size());
  if (res < 0) {
    throw DescriptiveFailureException(static_cast<ErrorCode>(res),
      description.data());
  }
}

}  // namespace internal
}  // namespace yogi
