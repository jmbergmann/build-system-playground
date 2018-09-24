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

namespace objects {
namespace detail {

BroadcastManager::BroadcastManager(ContextPtr context) : context_(context) {}

BroadcastManager::~BroadcastManager() {}

void BroadcastManager::SendBroadcast(api::Encoding enc,
                                     boost::asio::const_buffer data, bool retry,
                                     SendBroadcastHandler handler) {

                                     }

api::Result BroadcastManager::SendBroadcast(api::Encoding enc,
                                            boost::asio::const_buffer data,
                                            bool retry) {
  return api::kSuccess;
}

void BroadcastManager::ReceiveBroadcast(api::Encoding enc,
                                        boost::asio::mutable_buffer data,
                                        ReceiveBroadcastHandler handler) {}

void BroadcastManager::CancelReceiveBroadcast() {}

const LoggerPtr BroadcastManager::logger_ =
    Logger::CreateStaticInternalLogger("Branch.BroadcastManager");

}  // namespace detail
}  // namespace objects
