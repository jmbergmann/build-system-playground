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
#include "../objects/context.h"
#include "../objects/logger.h"
#include "../api/errors.h"
#include "types.h"

#include <boost/asio.hpp>
#include <memory>
#include <chrono>
#include <sstream>
#include <vector>
#include <functional>

namespace utils {

class TimedTcpSocket : public std::enable_shared_from_this<TimedTcpSocket> {
 public:
  typedef std::function<void(const api::Result&)> CompletionHandler;
  typedef std::function<void(const api::Result&, const ByteVector&)>
      ReceiveHandler;

  TimedTcpSocket(objects::ContextPtr context, std::chrono::nanoseconds timeout);
  virtual ~TimedTcpSocket();

  std::weak_ptr<TimedTcpSocket> MakeWeakPtr() { return shared_from_this(); }

  const boost::asio::ip::tcp::endpoint& GetRemoteEndpoint() const {
    return remote_ep_;
  }

  objects::ContextPtr GetContext() const { return context_; }
  bool HasBeenAccepted() const { return accepted_; }

  void Accept(boost::asio::ip::tcp::acceptor* acceptor,
              CompletionHandler handler);
  void Connect(const boost::asio::ip::tcp::endpoint& ep,
               CompletionHandler handler);
  void Send(SharedByteVector data, CompletionHandler handler);
  void ReceiveExactly(std::size_t num_bytes, ReceiveHandler handler);

 private:
  void SetNoDelayOption();
  void StartTimeout(std::weak_ptr<TimedTcpSocket> weak_self);
  void OnTimeout();

  static const objects::LoggerPtr logger_;

  const objects::ContextPtr context_;
  const std::chrono::nanoseconds timeout_;
  const SharedByteVector rcv_buffer_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::endpoint remote_ep_;
  bool accepted_;
  boost::asio::steady_timer timer_;
  bool timed_out_;
};

typedef std::shared_ptr<TimedTcpSocket> TimedTcpSocketPtr;
typedef std::weak_ptr<TimedTcpSocket> TimedTcpSocketWeakPtr;

}  // namespace utils

std::ostream& operator<<(std::ostream& os, const utils::TimedTcpSocket& socket);
