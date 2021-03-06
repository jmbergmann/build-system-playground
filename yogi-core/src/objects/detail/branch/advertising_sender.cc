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

#include "advertising_sender.h"
#include "../../../utils/algorithm.h"

#include <boost/asio/ip/multicast.hpp>

namespace objects {
namespace detail {

AdvertisingSender::AdvertisingSender(
    ContextPtr context, const boost::asio::ip::udp::endpoint& adv_ep)
    : context_(context),
      adv_ep_(adv_ep),
      timer_(context->IoContext()),
      active_send_ops_(0) {}

void AdvertisingSender::Start(LocalBranchInfoPtr info) {
  YOGI_ASSERT(!info_);

  info_ = info;
  for (auto& socket : sockets_) {
    YOGI_LOG_INFO(logger_, info_ << " Using interface " << socket->address
                                 << " for sending advertising messages.");
  }

  SetupSockets();
  SendAdvertisements();
}

void AdvertisingSender::SetupSockets() {
  for (auto& ifc : info_->GetAdvertisingInterfaces()) {
    for (auto& addr : ifc.addresses) {
      auto entry = std::make_shared<SocketEntry>(context_->IoContext());
      entry->interface_name = ifc.name;
      entry->address = addr;
      if (ConfigureSocket(entry)) {
        sockets_.push_back(entry);
      }
    }
  }
}

bool AdvertisingSender::ConfigureSocket(std::shared_ptr<SocketEntry> entry) {
  boost::system::error_code ec;
  entry->socket.open(adv_ep_.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }

  auto opt =
      entry->address.is_v6()
          ? boost::asio::ip::multicast::outbound_interface(
                static_cast<unsigned int>(entry->address.to_v6().scope_id()))
          : boost::asio::ip::multicast::outbound_interface(
                entry->address.to_v4());
  entry->socket.set_option(opt, ec);

  if (ec) {
    YOGI_LOG_ERROR(
        logger_,
        info_ << " Could not set outbound interface for socket using address "
              << entry->address << ": " << ec.message()
              << ". This interface will be ignored.");
    return false;
  }

  return true;
}

void AdvertisingSender::SendAdvertisements() {
  YOGI_ASSERT(active_send_ops_ == 0);

  if (sockets_.empty()) {
    YOGI_LOG_ERROR(logger_, info_ << " No network interfaces available for "
                                     "sending advertising messages.");
    return;
  }

  auto weak_self = std::weak_ptr<AdvertisingSender>{shared_from_this()};
  auto msg = info_->MakeAdvertisingMessage();

  for (const auto& socket : sockets_) {
    socket->socket.async_send_to(
        boost::asio::buffer(*msg), adv_ep_,
        [weak_self, msg, socket](auto ec, auto) {
          auto self = weak_self.lock();
          if (!self) return;

          if (ec) {
            YOGI_LOG_ERROR(logger_, self->info_
                                        << " Sending advertisement over "
                                        << socket->address
                                        << " failed: " << ec.message()
                                        << ". No more advertising messages "
                                           "will be sent over this interface.");

            self->sockets_.erase(utils::find(self->sockets_, socket));
          }

          --self->active_send_ops_;
          if (self->active_send_ops_ == 0) {
            self->StartTimer();
          }
        });
    ++active_send_ops_;
  }
}

void AdvertisingSender::StartTimer() {
  timer_.expires_after(info_->GetAdvertisingInterval());

  auto weak_self = std::weak_ptr<AdvertisingSender>(shared_from_this());
  timer_.async_wait([weak_self](auto ec) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!ec) {
      self->SendAdvertisements();
    } else {
      YOGI_LOG_ERROR(logger_,
                     self->info_
                         << " Awaiting advertising timer expiry failed: "
                         << ec.message()
                         << ". No more advertising messages will be sent.");
    }
  });
}

const LoggerPtr AdvertisingSender::logger_ =
    Logger::CreateStaticInternalLogger("Branch.AdvertisingSender");

}  // namespace detail
}  // namespace objects
