#ifndef YOGI_INTERNAL_ERROR_CODE_HELPERS_H
#define YOGI_INTERNAL_ERROR_CODE_HELPERS_H

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
  int res = fn(description.data(), static_cast<int>(description.size()));
  if (res < 0) {
    auto ec = static_cast<ErrorCode>(res);
    if (description[0] != '\0') {
      throw DescriptiveFailureException(ec, description.data());
    } else {
      throw FailureException(ec);
    }
  }
}

template <typename Fn>
inline void WithErrorCodeToResult(int res, Fn fn) {
  if (res < 0) {
    fn(yogi::Failure(static_cast<ErrorCode>(res)));
  } else {
    fn(yogi::Success(res));
  }
}

}  // namespace internal
}  // namespace yogi

#endif  // YOGI_INTERNAL_ERROR_CODE_HELPERS_H
