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

MessageTransport::MessageTransport(TransportPtr transport,
                                   std::size_t tx_queue_size,
                                   std::size_t rx_queue_size)
    : transport_(transport), tx_rb_(tx_queue_size), rx_rb_(rx_queue_size) {}

bool MessageTransport::CanSendImmediately(std::size_t msg_size) const {
  return true;
}

void MessageTransport::Send(boost::asio::const_buffer msg) {}

void MessageTransport::Receive(boost::asio::mutable_buffer msg,
                               ReceiveHandler handler) {}

void MessageTransport::CancelReceive() {}

}  // namespace network
