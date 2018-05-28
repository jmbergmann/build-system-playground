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
  socket_.async_send_to(
      boost::asio::buffer(info_->MakeAdvertisingMessage()), adv_ep_,
      [weak_self](auto ec, auto) {
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
