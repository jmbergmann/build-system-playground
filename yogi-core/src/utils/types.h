#pragma once

#include "../config.h"

#include <vector>
#include <memory>
#include <utility>

namespace utils {

typedef unsigned char Byte;
typedef std::vector<Byte> ByteVector;
typedef std::shared_ptr<ByteVector> SharedByteVector;

template <typename... Args>
SharedByteVector MakeSharedByteVector(Args&&... args) {
  return std::make_shared<ByteVector>(std::forward<Args>(args)...);
}

}  // namespace utils
