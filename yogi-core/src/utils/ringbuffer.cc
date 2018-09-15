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

#include "ringbuffer.h"

namespace utils {

LockFreeRingBuffer::LockFreeRingBuffer(std::size_t capacity)
    : capacity_(capacity) {
  write_idx_ = 0;
  read_idx_ = 0;
  data_.resize(capacity + 1);
}

bool LockFreeRingBuffer::empty() {
  auto wi = write_idx_.load(std::memory_order_relaxed);
  auto ri = read_idx_.load(std::memory_order_relaxed);
  return wi == ri;
}

bool LockFreeRingBuffer::full() {
  auto wi = write_idx_.load(std::memory_order_relaxed);
  auto ri = read_idx_.load(std::memory_order_acquire);
  return write_available(wi, ri) == 0;
}

char LockFreeRingBuffer::front() const {
  auto ri = read_idx_.load(std::memory_order_relaxed);
  return data_[ri];
}

void LockFreeRingBuffer::pop() {
  write_idx_.load(std::memory_order_acquire);  // TODO: Do we need this?
  auto ri = read_idx_.load(std::memory_order_relaxed);

  YOGI_ASSERT(!empty());

  auto next = next_index(ri);
  read_idx_.store(next, std::memory_order_release);
}

std::size_t LockFreeRingBuffer::read(Byte* buffer, std::size_t max_size) {
  auto wi = write_idx_.load(std::memory_order_acquire);
  auto ri = read_idx_.load(std::memory_order_relaxed);

  auto avail = read_available(wi, ri);
  if (avail == 0) {
    return 0;
  }

  max_size = std::min(max_size, avail);

  auto new_ri = ri + max_size;
  if (new_ri > data_.size()) {
    auto count_0 = data_.size() - ri;
    auto count_1 = max_size - count_0;

    std::copy(data_.begin() + ri, data_.begin() + data_.size(), buffer);
    std::copy(data_.begin(), data_.begin() + count_1, buffer + count_0);

    new_ri -= data_.size();
  } else {
    std::copy(data_.begin() + ri, data_.begin() + ri + max_size, buffer);

    if (new_ri == data_.size()) {
      new_ri = 0;
    }
  }

  read_idx_.store(new_ri, std::memory_order_release);
  return max_size;
}

void LockFreeRingBuffer::commit_first_read_array(std::size_t n) {
  YOGI_ASSERT(n <= boost::asio::buffer_size(first_read_array()));

  write_idx_.load(std::memory_order_acquire);  // TODO: Do we need this?
  auto ri = read_idx_.load(std::memory_order_relaxed);

  ri += n;
  if (ri == data_.size()) {
    ri = 0;
  }

  read_idx_.store(ri, std::memory_order_release);
}

boost::asio::const_buffers_1 LockFreeRingBuffer::first_read_array() const {
  auto wi = write_idx_.load(std::memory_order_relaxed);
  auto ri = read_idx_.load(std::memory_order_relaxed);

  if (wi < ri) {
    return boost::asio::buffer(data_.data() + ri, data_.size() - ri);
  } else {
    return boost::asio::buffer(data_.data() + ri, wi - ri);
  }
}

std::size_t LockFreeRingBuffer::write(const Byte* data, std::size_t size) {
  auto wi = write_idx_.load(std::memory_order_relaxed);
  auto ri = read_idx_.load(std::memory_order_acquire);

  auto avail = write_available(wi, ri);
  if (avail == 0) {
    return 0;
  }

  std::size_t input_cnt = size;
  input_cnt = std::min(input_cnt, avail);

  auto new_wi = wi + input_cnt;
  auto last = data + input_cnt;

  if (new_wi > data_.size()) {
    auto count_0 = data_.size() - wi;
    auto midpoint = data + count_0;

    std::uninitialized_copy(data, midpoint, data_.begin() + wi);
    std::uninitialized_copy(midpoint, last, data_.begin());

    new_wi -= data_.size();
  } else {
    std::uninitialized_copy(data, last, data_.begin() + wi);

    if (new_wi == data_.size()) {
      new_wi = 0;
    }
  }

  write_idx_.store(new_wi, std::memory_order_release);
  return input_cnt;
}

void LockFreeRingBuffer::commit_first_write_array(std::size_t n) {
  YOGI_ASSERT(n <= boost::asio::buffer_size(first_write_array()));

  auto wi = write_idx_.load(std::memory_order_relaxed);
  read_idx_.load(std::memory_order_acquire);  // TODO: do we need this?

  wi += n;
  if (wi >= data_.size()) {
    wi -= data_.size();
  }

  write_idx_.store(wi, std::memory_order_release);
}

boost::asio::mutable_buffers_1 LockFreeRingBuffer::first_write_array() {
  auto wi = write_idx_.load(std::memory_order_relaxed);
  auto ri = read_idx_.load(std::memory_order_relaxed);

  if (wi < ri) {
    return boost::asio::buffer(data_.data() + wi, ri - wi - 1);
  }

  return boost::asio::buffer(data_.data() + wi,
                             data_.size() - wi - (ri == 0 ? 1 : 0));
}

std::size_t LockFreeRingBuffer::read_available(std::size_t write_idx,
                                               std::size_t read_idx) const {
  if (write_idx >= read_idx) {
    return write_idx - read_idx;
  }

  return write_idx + data_.size() - read_idx;
}

std::size_t LockFreeRingBuffer::write_available(std::size_t write_idx,
                                                std::size_t read_idx) const {
  auto n = read_idx - write_idx - 1;
  if (write_idx >= read_idx) {
    n += data_.size();
  }

  return n;
}

std::size_t LockFreeRingBuffer::next_index(std::size_t idx) const {
  idx += 1;
  if (idx >= data_.size()) {
    idx -= data_.size();
  }

  return idx;
}
}  // namespace utils
