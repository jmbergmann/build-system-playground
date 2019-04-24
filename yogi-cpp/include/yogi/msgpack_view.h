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

#ifndef YOGI_MSGPACK_VIEW_H
#define YOGI_MSGPACK_VIEW_H

//! \file
//!
//! Helpers for passing MessagePack-encoded data to functions.

#include "msgpack.h"

#include <string>
#include <sstream>

namespace yogi {

////////////////////////////////////////////////////////////////////////////////
/// Helper class for passing different types of MessagePack data to functions
/// that internally require a buffer holding the serialized MessagePack data.
///
/// \attention
///   It is imperative that the value passed to any of the view's constructors
///   outlives the view object!
////////////////////////////////////////////////////////////////////////////////
class MsgpackView {
 public:
  /// Constructs a view that evaluates to a nullptr.
  MsgpackView() : data_(nullptr), size_(0) {}

  /// Constructs a view from a buffer.
  ///
  /// \param data Buffer to use.
  /// \param size Size of the buffer in bytes.
  MsgpackView(const void* data, int size) : data_(data), size_(size){};

  /// Constructs a view from a buffer.
  ///
  /// \param data Buffer to use.
  /// \param size Size of the buffer in bytes.
  MsgpackView(const void* data, std::size_t size)
      : MsgpackView(data, static_cast<int>(size)){};

  /// Constructs a view from a standard string.
  ///
  /// \param s Referenced string.
  MsgpackView(const std::string& s)
      : data_(s.c_str()), size_(static_cast<int>(s.size())) {}

  /// Constructs a view from a std::stringstream.
  ///
  /// \param ss The string stream to reference.
  MsgpackView(const std::stringstream& ss)
      : tmp_(ss.str()),
        data_(tmp_.c_str()),
        size_(static_cast<int>(tmp_.size())){};

  /// Returns a buffer holding the serialized MessagePack data.
  ///
  /// \attention
  ///   The returned value is only valid as long as both the view object and the
  ///   parameter passed to any of its constructors are valid.
  ///
  /// \returns Buffer holding the serialized MessagePack data.
  const void* Data() const { return data_; }

  /// Returns the length of the serialized MessagePack data in bytes.
  ///
  /// \returns Size of the serialized MessagePack data in bytes.
  int Size() const { return size_; }

 private:
  const std::string tmp_;
  const void* const data_;
  const int size_;
};

}  // namespace yogi

#endif  // YOGI_MSGPACK_VIEW_H
