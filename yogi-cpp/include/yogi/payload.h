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

#include "json_view.h"

namespace yogi {

////////////////////////////////////////////////////////////////////////////////
/// Encoding types.
///
/// Possible data/payload encoding types.
////////////////////////////////////////////////////////////////////////////////
enum class EncodingType {
  /// Data is encoded as JSON.
  kJson = 0,

  /// Data is encoded as MessagePack
  kMsgPack = 1,
};

class Payload final {
 public:
  Payload(const void* data, int size, EncodingType enc)
      : data_(data), size_(size), enc_(enc) {}

  Payload(const JsonView& json)
      : data_(json.Data()), size_(json.Size()), enc_(EncodingType::kJson) {}

  const void* Data() const { return data_; }
  int Size() const { return size_; };
  EncodingType Encoding() const { return enc_; }

 private:
  const void* data_;
  int size_;
  EncodingType enc_;
};

}  // namespace yogi

#endif  // YOGI_PAYLOAD_H
