/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef YOGI_INTERNAL_ERROR_CODE_HELPERS_H
#define YOGI_INTERNAL_ERROR_CODE_HELPERS_H

//! \file
//!
//! Helpers for handling error codes.

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
