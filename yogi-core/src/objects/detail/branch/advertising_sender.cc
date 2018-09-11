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

namespace objects {
namespace detail {

AdvertisingSender::AdvertisingSender(
    ContextPtr context, const boost::asio::ip::udp::endpoint& adv_ep)
    : context_(context),
      adv_ep_(adv_ep),
      socket_(context->IoContext()),
      timer_(context->IoContext()) {
  SetupSocket();
}

void AdvertisingSender::Start(BranchInfoPtr info) {
  YOGI_ASSERT(!info_);

  info_ = info;
  SendAdvertisement();
}

void AdvertisingSender::SetupSocket() {
  boost::system::error_code ec;
  socket_.open(adv_ep_.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }
}

void AdvertisingSender::SendAdvertisement() {
  auto weak_self = std::weak_ptr<AdvertisingSender>{shared_from_this()};
  auto msg = info_->MakeAdvertisingMessage();
  socket_.async_send_to(
      boost::asio::buffer(*msg), adv_ep_, [weak_self, msg](auto ec, auto) {
        auto self = weak_self.lock();
        if (!self) return;

        if (!ec) {
          self->StartTimer();
        } else {
          YOGI_LOG_ERROR(
              logger_, self->info_
                           << " Sending advertisement failed: " << ec.message()
                           << ". No more advertising messages will be sent.");
        }
      });
}

void AdvertisingSender::StartTimer() {
  timer_.expires_after(info_->GetAdvertisingInterval());

  auto weak_self = std::weak_ptr<AdvertisingSender>(shared_from_this());
  timer_.async_wait([weak_self](auto ec) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!ec) {
      self->SendAdvertisement();
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
