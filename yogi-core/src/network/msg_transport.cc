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

#include "msg_transport.h"

namespace network {
namespace internal {

std::size_t SerializeMsgSizeField(std::size_t msg_size,
                                  std::array<utils::Byte, 5>* buffer) {
  int length = 1 + (msg_size >= (1 << 7)) + (msg_size >= (1 << 14)) +
               (msg_size >= (1 << 21)) + (msg_size >= (1 << 28));

  auto it = buffer->begin();
  for (int i = length; i > 0; --i) {
    auto byte = static_cast<utils::Byte>((msg_size >> ((i - 1) * 7)) & 0x7F);
    byte |= (i > 1 ? (1 << 7) : 0);
    *it++ = byte;
  }

  return length;
}

bool DeserializeMsgSizeField(const std::array<utils::Byte, 5>& buffer,
                             std::size_t size, std::size_t* msg_size) {
  YOGI_ASSERT(size <= buffer.size());

  std::size_t tmp = 0;

  for (std::size_t i = 0; i < size; ++i) {
    auto byte = buffer[i];

    tmp |= static_cast<std::size_t>(byte & ~(1 << 7));
    if (!(byte & (1 << 7))) {
      *msg_size = tmp;
      return true;
    }

    tmp <<= 7;
  }

  return false;
}

}  // namespace internal

MessageTransport::MessageTransport(TransportPtr transport,
                                   std::size_t tx_queue_size,
                                   std::size_t rx_queue_size)
    : transport_(transport), tx_rb_(tx_queue_size), rx_rb_(rx_queue_size) {}

bool MessageTransport::CanSendImmediately(std::size_t msg_size) const {
  // Note: We use 5 here as the maximum length of the serialized msg_size value
  //       instead of actually calculating the length. Nobody gives a shit about
  //       those few bytes anyway.
  return msg_size + 5 <= tx_rb_.AvailableForWrite();
}

void MessageTransport::Send(boost::asio::const_buffer msg) {}

void MessageTransport::Receive(boost::asio::mutable_buffer msg,
                               ReceiveHandler handler) {}

void MessageTransport::CancelReceive() {}

}  // namespace network
