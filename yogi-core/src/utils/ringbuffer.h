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

#pragma once

#include "../config.h"
#include "types.h"

#include <boost/asio/buffer.hpp>
#include <atomic>
#include <vector>
#include <memory>

namespace utils {

// Implementation based on the lock-free single-producer/single-consumer
// ringbuffer implementation in boost by Tim Blechmann (spsc_queue).
class LockFreeRingBuffer {
 public:
  explicit LockFreeRingBuffer(std::size_t capacity);

  std::size_t capacity() const { return capacity_; };
  bool empty();
  bool full();
  char front() const;
  void pop();
  std::size_t read(Byte* buffer, std::size_t max_size);
  void commit_first_read_array(std::size_t n);
  boost::asio::const_buffers_1 first_read_array() const;
  std::size_t write(const Byte* data, std::size_t size);
  void commit_first_write_array(std::size_t n);
  boost::asio::mutable_buffers_1 first_write_array();

 private:
  std::size_t read_available(std::size_t write_idx, std::size_t read_idx) const;
  std::size_t write_available(std::size_t write_idx,
                              std::size_t read_idx) const;
  std::size_t next_index(std::size_t idx) const;

  static constexpr int kCacheLineSize = 64;
  std::atomic<std::size_t> write_idx_;
  char padding_[kCacheLineSize - sizeof(std::size_t)];
  std::atomic<std::size_t> read_idx_;
  const std::size_t capacity_;
  ByteVector data_;
};

}  // namespace utils
