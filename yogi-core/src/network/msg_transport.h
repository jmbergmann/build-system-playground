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
#include "../utils/ringbuffer.h"
#include "transport.h"

#include <boost/asio/buffer.hpp>
#include <functional>
#include <memory>
#include <array>

namespace network {
namespace internal {

std::size_t SerializeMsgSizeField(std::size_t msg_size,
                                  std::array<utils::Byte, 5>* buffer);
bool DeserializeMsgSizeField(const std::array<utils::Byte, 5>& buffer,
                             std::size_t size, std::size_t* msg_size);

}  // namespace internal

class MessageTransport;
typedef std::shared_ptr<MessageTransport> MessageTransportPtr;
typedef std::weak_ptr<MessageTransport> MessageTransportWeakPtr;

class MessageTransport : public std::enable_shared_from_this<MessageTransport> {
 public:
  typedef std::function<void(const api::Result&, std::size_t msg_size)>
      ReceiveHandler;

  MessageTransport(TransportPtr transport, std::size_t tx_queue_size,
                   std::size_t rx_queue_size);

  bool CanSendImmediately(std::size_t msg_size) const;
  void Send(boost::asio::const_buffer msg);
  void Receive(boost::asio::mutable_buffer msg, ReceiveHandler handler);
  void CancelReceive();

 private:
  typedef std::array<utils::Byte, 5> SizeFieldBuffer;

  MessageTransportWeakPtr MakeWeakPtr() { return shared_from_this(); }

  const TransportPtr transport_;
  utils::LockFreeRingBuffer tx_rb_;
  utils::LockFreeRingBuffer rx_rb_;
};

}  // namespace network
