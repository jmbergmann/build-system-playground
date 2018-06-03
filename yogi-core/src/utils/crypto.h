#pragma once

#include "../config.h"
#include "types.h"

#include <vector>

namespace utils {

ByteVector MakeSha256(const ByteVector& data);
ByteVector GenerateRandomBytes(std::size_t n);

}  // namespace utils
