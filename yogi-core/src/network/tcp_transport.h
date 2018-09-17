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
#include "transport.h"

#include <boost/asio.hpp>

namespace network {

class TcpTransport;
typedef std::shared_ptr<TcpTransport> TcpTransportPtr;
typedef std::weak_ptr<TcpTransport> TcpTransportWeakPtr;

class TcpTransport : public Transport {
 public:
  typedef std::function<void(const api::Result&, TcpTransportPtr)>
      CreateHandler;

  static void Accept(objects::ContextPtr context,
                     boost::asio::ip::tcp::acceptor* acceptor,
                     std::chrono::nanoseconds timeout, CreateHandler handler);
  static void Connect(objects::ContextPtr context,
                      const boost::asio::ip::tcp::endpoint& ep,
                      std::chrono::nanoseconds timeout, CreateHandler handler);

  bool CreatedViaAccept() const { return created_via_accept_; }

 protected:
  virtual void WriteSome(boost::asio::const_buffer data,
                         CompletionHandler handler) override;
  virtual void ReadSome(boost::asio::mutable_buffer data,
                        CompletionHandler handler) override;
  virtual void Shutdown() override;

 private:
  static std::string MakePeerDescription(
      const boost::asio::ip::tcp::socket& socket);
  static void CloseSocket(boost::asio::ip::tcp::socket* s);

  TcpTransport(objects::ContextPtr context,
               boost::asio::ip::tcp::socket&& socket,
               std::chrono::nanoseconds timeout, bool created_via_accept);
  void SetNoDelayOption();

  static const objects::LoggerPtr logger_;
  boost::asio::ip::tcp::socket socket_;
  const bool created_via_accept_;
};

}  // namespace network
