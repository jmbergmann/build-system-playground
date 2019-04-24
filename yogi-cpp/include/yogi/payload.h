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

#ifndef YOGI_PAYLOAD_H
#define YOGI_PAYLOAD_H

//! \file
//!
//! User payload.

#include <memory>
#include <vector>

namespace yogi {

////////////////////////////////////////////////////////////////////////////////
/// Represents a buffer holding user-defined payload encoded in either JSON or
/// MessagePack.
////////////////////////////////////////////////////////////////////////////////
class Payload {
 public:
  static constexpr int kDefaultCapacity = 1024;

  Payload(int capacity = kDefaultCapacity)
      : Payload(static_cast<std::size_t>(capacity)) {}

  Payload(std::size_t capacity) { data_.reserve(capacity); }

 private:
  std::vector<char> data_;
};

/// Shared pointer to a user payload object.
typedef std::shared_ptr<Payload> PayloadPtr;

}  // namespace yogi

#endif  // YOGI_PAYLOAD_H
