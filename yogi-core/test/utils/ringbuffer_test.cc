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

#include "../common.h"
#include "../../src/utils/ringbuffer.h"
using namespace utils;

class RingBufferTest : public Test {
 protected:
  LockFreeRingBuffer uut{10};

  Byte get_first_read_array_element(std::size_t idx) const {
    ByteVector data(idx + 1);
    boost::asio::buffer_copy(boost::asio::buffer(data), uut.first_read_array());
    return data.back();
  }

  void set_first_write_array_element(std::size_t idx, Byte byte) {
    boost::asio::buffer_copy(uut.first_write_array() + idx,
                             boost::asio::buffer(&byte, 1));
  }

  std::size_t first_read_array_size() const {
    return boost::asio::buffer_size(uut.first_read_array());
  }

  std::size_t first_write_array_size() {
    return boost::asio::buffer_size(uut.first_write_array());
  }
};

TEST_F(RingBufferTest, FirstReadArray) {
  ByteVector data(uut.capacity(), 0);
  data[0] = 1;
  data[1] = 2;
  data[2] = 3;
  data.back() = 4;

  auto n = uut.write(data.data(), data.size());
  EXPECT_EQ(n, data.size());
  EXPECT_EQ(uut.capacity(), first_read_array_size());
  EXPECT_EQ(1, get_first_read_array_element(0));
  EXPECT_EQ(4, get_first_read_array_element(uut.capacity() - 1));

  uut.commit_first_read_array(2);
  EXPECT_EQ(uut.capacity() - 2, first_read_array_size());
  EXPECT_EQ(3, get_first_read_array_element(0));
  EXPECT_EQ(4, get_first_read_array_element(uut.capacity() - 3));

  uut.write(data.data(), 2);
  EXPECT_EQ(uut.capacity() - 1, first_read_array_size());
  EXPECT_EQ(3, get_first_read_array_element(0));
  EXPECT_EQ(4, get_first_read_array_element(uut.capacity() - 3));
  EXPECT_EQ(1, get_first_read_array_element(uut.capacity() - 2));

  uut.commit_first_read_array(uut.capacity() - 1);
  EXPECT_EQ(1, first_read_array_size());
  EXPECT_EQ(2, get_first_read_array_element(0));
}

TEST_F(RingBufferTest, FirstWriteArray) {
  EXPECT_EQ(uut.capacity(), first_write_array_size());
  set_first_write_array_element(0, 1);
  set_first_write_array_element(1, 2);
  uut.commit_first_write_array(2);

  Byte data[2];
  EXPECT_EQ(2, uut.read(data, 2));
  EXPECT_EQ(1, data[0]);
  EXPECT_EQ(2, data[1]);

  EXPECT_EQ(uut.capacity() - 1, first_write_array_size());
  uut.commit_first_write_array(first_write_array_size());
  EXPECT_EQ(1, first_write_array_size());
  uut.commit_first_write_array(1);
  EXPECT_EQ(0, first_write_array_size());
}

TEST_F(RingBufferTest, Empty) {
  EXPECT_TRUE(uut.empty());
  ByteVector buffer{'x'};
  uut.write(buffer.data(), buffer.size());
  EXPECT_FALSE(uut.empty());
  Byte byte;
  uut.read(&byte, 1);
  EXPECT_TRUE(uut.empty());
}

TEST_F(RingBufferTest, Full) {
  EXPECT_FALSE(uut.full());
  ByteVector buffer(uut.capacity() - 1, 'x');
  uut.write(buffer.data(), buffer.size());
  EXPECT_FALSE(uut.full());
  uut.write(buffer.data(), 1);
  Byte byte;
  uut.read(&byte, 1);
  EXPECT_FALSE(uut.full());
}

TEST_F(RingBufferTest, FrontAndPop) {
  ByteVector buffer{'a', 'b', 'c'};
  uut.write(buffer.data(), buffer.size());
  EXPECT_EQ('a', uut.front());
  uut.pop();
  EXPECT_EQ('b', uut.front());
  uut.pop();
  EXPECT_EQ('c', uut.front());
  uut.pop();
  EXPECT_TRUE(uut.empty());
}
