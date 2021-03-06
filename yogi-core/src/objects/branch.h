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
#include "context.h"
#include "detail/branch/broadcast_manager.h"
#include "detail/branch/connection_manager.h"

namespace objects {

class Branch : public api::ExposedObjectT<Branch, api::ObjectType::kBranch> {
 public:
  using BranchEventHandler = detail::ConnectionManager::BranchEventHandler;
  using SendBroadcastHandler = detail::BroadcastManager::SendBroadcastHandler;
  using ReceiveBroadcastHandler =
      detail::BroadcastManager::ReceiveBroadcastHandler;
  using BranchInfoStringsList =
      detail::ConnectionManager::BranchInfoStringsList;
  using SendBroadcastOperationId =
      detail::BroadcastManager::SendBroadcastOperationId;

  Branch(ContextPtr context, std::string name, std::string description,
         std::string net_name, std::string password, std::string path,
         const std::vector<std::string>& adv_if_strings,
         const boost::asio::ip::udp::endpoint& adv_ep,
         std::chrono::nanoseconds adv_interval,
         std::chrono::nanoseconds timeout, bool ghost_mode,
         std::size_t tx_queue_size, std::size_t rx_queue_size,
         std::size_t transceive_byte_limit);

  void Start();

  const boost::uuids::uuid& GetUuid() const;
  std::string MakeInfoString() const;
  BranchInfoStringsList MakeConnectedBranchesInfoStrings() const;
  void AwaitEventAsync(api::BranchEvents events, BranchEventHandler handler);
  bool CancelAwaitEvent();
  SendBroadcastOperationId SendBroadcastAsync(const network::Payload& payload,
                                              bool retry,
                                              SendBroadcastHandler handler);
  api::Result SendBroadcast(const network::Payload& payload, bool block);
  bool CancelSendBroadcast(SendBroadcastOperationId oid);
  void ReceiveBroadcast(api::Encoding enc, boost::asio::mutable_buffer data,
                        ReceiveBroadcastHandler handler);
  bool CancelReceiveBroadcast();

 private:
  void OnConnectionChanged(const api::Result& res,
                           const detail::BranchConnectionPtr& conn);
  void OnMessageReceived(const network::IncomingMessage& msg,
                         const detail::BranchConnectionPtr& conn);

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const detail::ConnectionManagerPtr connection_manager_;
  const detail::LocalBranchInfoPtr info_;
  const detail::BroadcastManagerPtr broadcast_manager_;
};

typedef std::shared_ptr<Branch> BranchPtr;

}  // namespace objects
