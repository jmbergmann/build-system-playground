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
#include "connection_manager.h"

#include <boost/asio/buffer.hpp>
#include <vector>
#include <mutex>

namespace objects {
namespace detail {

class BroadcastManager final
    : public std::enable_shared_from_this<BroadcastManager> {
 public:
  typedef network::MessageTransport::OperationTag SendBroadcastOperationId;
  typedef std::function<void(const api::Error& res,
                             SendBroadcastOperationId oid)>
      SendBroadcastHandler;
  typedef std::function<void(const api::Error& res, std::size_t size)>
      ReceiveBroadcastHandler;

  BroadcastManager(ContextPtr context, ConnectionManager& conn_manager);
  virtual ~BroadcastManager();

  api::Result SendBroadcast(api::Encoding enc, boost::asio::const_buffer data,
                            bool retry);

  SendBroadcastOperationId SendBroadcastAsync(api::Encoding enc,
                                              boost::asio::const_buffer data,
                                              bool retry,
                                              SendBroadcastHandler handler);

  bool CancelSendBroadcast(SendBroadcastOperationId oid);

  void ReceiveBroadcast(api::Encoding enc, boost::asio::mutable_buffer data,
                        ReceiveBroadcastHandler handler);

  bool CancelReceiveBroadcast();

 private:
  static const LoggerPtr logger_;

  const ContextPtr context_;
  ConnectionManager& conn_manager_;
  std::mutex mutex_;
  std::vector<SendBroadcastOperationId> oids_;
};

typedef std::shared_ptr<BroadcastManager> BroadcastManagerPtr;

}  // namespace detail
}  // namespace objects
