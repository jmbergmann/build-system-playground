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

#include <boost/asio.hpp>
#include <memory>
#include <functional>
#include <sstream>
#include <chrono>
#include <string>

namespace network {

class Transport;
typedef std::shared_ptr<Transport> TransportPtr;
typedef std::weak_ptr<Transport> TransportWeakPtr;

class Transport : public std::enable_shared_from_this<Transport> {
 public:
  typedef std::function<void(const api::Result&,
                             const std::size_t bytes_transferred)>
      CompletionHandler;

  Transport(objects::ContextPtr context, std::chrono::nanoseconds timeout,
            std::string peer_description);
  virtual ~Transport();

  objects::ContextPtr GetContext() const { return context_; }
  const std::string& GetPeerDescription() const { return peer_description_; }

  void SendSome(boost::asio::const_buffer data, CompletionHandler handler);
  void SendAll(boost::asio::const_buffer data, CompletionHandler handler);
  void ReceiveSome(boost::asio::mutable_buffer data, CompletionHandler handler);
  void ReceiveAll(boost::asio::mutable_buffer data, CompletionHandler handler);
  void Close();

 protected:
  virtual void WriteSome(boost::asio::const_buffer data,
                         CompletionHandler handler) = 0;
  virtual void ReadSome(boost::asio::mutable_buffer data,
                        CompletionHandler handler) = 0;
  virtual void Shutdown() = 0;

 private:
  TransportWeakPtr MakeWeakPtr() { return shared_from_this(); }
  void SendAllImpl(boost::asio::const_buffer data, const api::Result& res,
                   std::size_t bytes_written, std::size_t total_bytes_written,
                   CompletionHandler handler);
  void ReceiveAllImpl(boost::asio::mutable_buffer data, const api::Result& res,
                      std::size_t bytes_read, std::size_t total_bytes_read,
                      CompletionHandler handler);
  void StartTimeout(boost::asio::steady_timer* timer,
                    TransportWeakPtr weak_self);
  void OnTimeout();

  const objects::ContextPtr context_;
  const std::chrono::nanoseconds timeout_;
  const std::string peer_description_;
  boost::asio::steady_timer tx_timer_;
  boost::asio::steady_timer rx_timer_;
  bool timed_out_;
  YOGI_DEBUG_ONLY(bool close_called_ = false;)
};

}  // namespace network

std::ostream& operator<<(std::ostream& os, const network::Transport& transport);
