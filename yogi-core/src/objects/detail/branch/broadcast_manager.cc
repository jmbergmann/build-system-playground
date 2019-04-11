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
#include "../../../utils/algorithm.h"

namespace objects {
namespace detail {

BroadcastManager::BroadcastManager(ContextPtr context,
                                   ConnectionManager& conn_manager)
    : context_(context), conn_manager_(conn_manager) {}

BroadcastManager::~BroadcastManager() {}

api::Result BroadcastManager::SendBroadcast(const network::UserData& user_data,
                                            bool block) {
  api::Result result;
  SendBroadcastAsync(user_data, block, [&](auto& res, auto) {
    result = res;
    this->tx_sync_cv_.notify_all();
  });

  std::unique_lock<std::mutex> lock(tx_sync_mutex_);
  tx_sync_cv_.wait(lock, [&] { return result != api::Result(); });

  return result;
}

BroadcastManager::SendBroadcastOperationId BroadcastManager::SendBroadcastAsync(
    const network::UserData& user_data, bool retry,
    SendBroadcastHandler handler) {
  network::messages::BroadcastOutgoing msg(user_data);

  auto oid = conn_manager_.MakeOperationId();

  if (retry) {
    std::shared_ptr<int> pending_handlers;
    conn_manager_.ForeachRunningSession([&](auto& conn) {
      this->SendNowOrLater(&pending_handlers, &msg, conn, handler, oid);
    });

    StoreOidForLaterOrCallHandlerNow(pending_handlers, handler, oid);
  } else {
    bool all_sent = true;
    conn_manager_.ForeachRunningSession([&](auto& conn) {
      if (!conn->TrySend(msg)) {
        all_sent = false;
      }
    });

    if (all_sent) {
      context_->Post([=] { handler(api::kSuccess, oid); });
    } else {
      context_->Post([=] { handler(api::Error(YOGI_ERR_TX_QUEUE_FULL), oid); });
    }
  }

  return oid;
}

bool BroadcastManager::CancelSendBroadcast(SendBroadcastOperationId oid) {
  {
    std::lock_guard<std::mutex> lock(tx_oids_mutex_);
    auto it = utils::find(tx_active_oids_, oid);
    if (it == tx_active_oids_.end()) return false;
    tx_active_oids_.erase(it);
  }

  bool canceled = false;
  conn_manager_.ForeachRunningSession(
      [&](auto& conn) { canceled |= conn->CancelSend(oid); });

  return canceled;
}

void BroadcastManager::ReceiveBroadcast(api::Encoding enc,
                                        boost::asio::mutable_buffer data,
                                        ReceiveBroadcastHandler handler) {
  YOGI_ASSERT(handler);

  std::lock_guard<std::recursive_mutex> lock(rx_mutex_);

  if (rx_handler_) {
    auto old_handler = rx_handler_;
    context_->Post([=] { old_handler(api::Error(YOGI_ERR_CANCELED), 0); });
  }

  rx_enc_ = enc;
  rx_data_ = data;
  rx_handler_ = handler;
}

bool BroadcastManager::CancelReceiveBroadcast() {
  std::lock_guard<std::recursive_mutex> lock(rx_mutex_);

  if (rx_handler_) {
    auto handler = rx_handler_;
    rx_handler_ = {};
    context_->Post([=] { handler(api::Error(YOGI_ERR_CANCELED), 0); });
    return true;
  }

  return false;
}

void BroadcastManager::OnBroadcastReceived(
    const network::messages::BroadcastIncoming& msg) {
  std::lock_guard<std::recursive_mutex> lock(rx_mutex_);

  if (rx_handler_) {
    auto handler = rx_handler_;
    rx_handler_ = {};
    std::size_t n = 0;
    auto res = msg.GetUserData().SerializeToUserBuffer(rx_data_, rx_enc_, &n);
    handler(res, n);  // TODO
  }
}

void BroadcastManager::SendNowOrLater(SharedCounter* pending_handlers,
                                      network::OutgoingMessage* msg,
                                      BranchConnectionPtr conn,
                                      SendBroadcastHandler handler,
                                      SendBroadcastOperationId oid) {
  try {
    if (!conn->TrySend(*msg)) {
      if (!*pending_handlers) {
        *pending_handlers = std::make_shared<int>(1);
      } else {
        ++**pending_handlers;
      }

      try {
        auto weak_self = std::weak_ptr<BroadcastManager>{shared_from_this()};
        conn->SendAsync(msg, [=](auto& res) {
          if (--**pending_handlers == 0) {
            auto self = weak_self.lock();
            if (self) {
              if (self->RemoveActiveOid(oid)) {
                handler(api::kSuccess, oid);
              } else {
                handler(api::Error(YOGI_ERR_CANCELED), oid);
              }
            } else {
              handler(api::Error(YOGI_ERR_CANCELED), oid);
            }
          }
        });
      } catch (...) {
        --**pending_handlers;
        throw;
      }
    }
  } catch (const api::Error& err) {
    YOGI_LOG_ERROR(logger_,
                   "Could not send broadcast to " << conn << ": " << err);
  }
}

void BroadcastManager::StoreOidForLaterOrCallHandlerNow(
    SharedCounter pending_handlers, SendBroadcastHandler handler,
    SendBroadcastOperationId oid) {
  if (pending_handlers) {
    std::lock_guard<std::mutex> lock(tx_oids_mutex_);
    tx_active_oids_.push_back(oid);
  } else {
    context_->Post([=] { handler(api::kSuccess, oid); });
  }
}

bool BroadcastManager::RemoveActiveOid(SendBroadcastOperationId oid) {
  std::lock_guard<std::mutex> lock(tx_oids_mutex_);
  auto it = utils::find(tx_active_oids_, oid);
  if (it != tx_active_oids_.end()) {
    tx_active_oids_.erase(it);
    return true;
  }

  return false;
}

const LoggerPtr BroadcastManager::logger_ =
    Logger::CreateStaticInternalLogger("Branch.BroadcastManager");

}  // namespace detail
}  // namespace objects
