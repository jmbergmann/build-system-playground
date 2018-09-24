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

#include "../../../config.h"
#include "../../context.h"
#include "../../logger.h"

#include <boost/asio/buffer.hpp>

namespace objects {
namespace detail {

class BroadcastManager final
    : public std::enable_shared_from_this<BroadcastManager> {
 public:
  typedef std::function<void(const api::Error& res)> SendBroadcastHandler;
  typedef std::function<void(const api::Error& res, std::size_t size)>
      ReceiveBroadcastHandler;

  BroadcastManager(ContextPtr context);
  virtual ~BroadcastManager();

  void SendBroadcastAsync(api::Encoding enc, boost::asio::const_buffer data,
                          bool retry, SendBroadcastHandler handler);

  api::Result SendBroadcast(api::Encoding enc, boost::asio::const_buffer data,
                            bool retry);

  void CancelSendBroadcast();

  void ReceiveBroadcast(api::Encoding enc, boost::asio::mutable_buffer data,
                        ReceiveBroadcastHandler handler);

  void CancelReceiveBroadcast();

 private:
  static const LoggerPtr logger_;

  const ContextPtr context_;
};

typedef std::shared_ptr<BroadcastManager> BroadcastManagerPtr;

}  // namespace detail
}  // namespace objects
