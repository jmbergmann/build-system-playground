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
#include "../../src/network/msg_transport.h"

// class LoopbackTransport : public network::Transport {
//  public:
//   LoopbackTransport(objects::ContextPtr context, std::chrono::nanoseconds timeout) {

//   }
// };

class MessageTransportTest : public TestFixture {
 protected:
};

TEST_F(MessageTransportTest, MsgSizeFieldSerialization) {
  using namespace network::internal;

  struct Entry {
    std::size_t ser_length;
    std::size_t value;
  };

  // those are all the corner cases
  const Entry entries[] = {
      // clang-format off
      {1, 0ul},
      {1, 127ul},
      {2, 128ul},
      {2, 16383ul},
      {3, 16384ul},
      {3, 2097151ul},
      {4, 2097152ul},
      {4, 268435455ul},
      {5, 268435456ul},
      {5, 4294967295ul},
      // clang-format on
  };

  for (auto entry : entries) {
    std::array<utils::Byte, 5> buffer = {0};

    auto n = SerializeMsgSizeField(entry.value, &buffer);
    EXPECT_EQ(n, entry.ser_length);

    std::size_t des_value;
    for (std::size_t i = 1; i < entry.ser_length; ++i) {
      EXPECT_FALSE(DeserializeMsgSizeField(buffer, i, &des_value));
    }

    EXPECT_TRUE(DeserializeMsgSizeField(buffer, entry.ser_length, &des_value));
    EXPECT_EQ(des_value, entry.value);
  }
}

TEST_F(MessageTransportTest, CanSendImmediately) {
}
