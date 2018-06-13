#pragma once

#include "../config.h"

#include <vector>
#include <memory>
#include <utility>

#define YOGI_DEFINE_FLAG_OPERATORS(enum_)                                 \
  inline enum_ operator|(enum_ a, enum_ b) {                              \
    return static_cast<enum_>(static_cast<int>(a) | static_cast<int>(b)); \
  }                                                                       \
  inline enum_ operator&(enum_ a, enum_ b) {                              \
    return static_cast<enum_>(static_cast<int>(a) & static_cast<int>(b)); \
  }

namespace utils {

typedef unsigned char Byte;
typedef std::vector<Byte> ByteVector;
typedef std::shared_ptr<ByteVector> SharedByteVector;

template <typename... Args>
SharedByteVector MakeSharedByteVector(Args&&... args) {
  return std::make_shared<ByteVector>(std::forward<Args>(args)...);
}

}  // namespace utils
