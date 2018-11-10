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

#include "broadcast_manager.h"

#include <algorithm>

namespace objects {
namespace detail {

BroadcastManager::BroadcastManager(ContextPtr context) : context_(context) {}

BroadcastManager::~BroadcastManager() {}

api::Result BroadcastManager::SendBroadcast(api::Encoding enc,
                                            boost::asio::const_buffer data,
                                            bool retry) {
  return api::kSuccess;
}

BroadcastManager::SendBroadcastOperationId BroadcastManager::SendBroadcastAsync(
    api::Encoding enc, boost::asio::const_buffer data, bool retry,
    SendBroadcastHandler handler) {
  return 0;
}

bool BroadcastManager::CancelSendBroadcast(SendBroadcastOperationId oid) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = std::find(oids_.begin(), oids_.end(), oid);
  if (it == oids_.end()) return false;
  oids_.erase(it);

  // TODO: cancel async msg in all MesssageTransport instances

  return true;
}

void BroadcastManager::ReceiveBroadcast(api::Encoding enc,
                                        boost::asio::mutable_buffer data,
                                        ReceiveBroadcastHandler handler) {}

bool BroadcastManager::CancelReceiveBroadcast() { return false; }

const LoggerPtr BroadcastManager::logger_ =
    Logger::CreateStaticInternalLogger("Branch.BroadcastManager");

}  // namespace detail
}  // namespace objects
