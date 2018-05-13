#include "adv_sender.h"
#include "../../../api/error.h"

namespace objects {
namespace detail {

const LoggerPtr AdvSender::logger_ =
    Logger::CreateStaticInternalLogger("Branch");

AdvSender::AdvSender(ContextPtr context, LocalBranchInfoPtr info)
    : context_(context),
      info_(info),
      socket_(context->IoContext()),
      timer_(context->IoContext()) {
  SetupSocket();
}

void AdvSender::Start() {
  message_ = info_->MakeAdvertisingMessage();
  SendAdvertisement();
}

void AdvSender::SetupSocket() {
  boost::system::error_code ec;
  socket_.open(info_->adv_ep.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }
}

void AdvSender::SendAdvertisement() {
  auto weak_self = std::weak_ptr<AdvSender>{shared_from_this()};
  socket_.async_send_to(
      boost::asio::buffer(message_), info_->adv_ep, [weak_self](auto ec, auto) {
        auto self = weak_self.lock();
        if (!self) return;

        if (!ec) {
          self->StartTimer();
        } else {
          YOGI_LOG_ERROR(self->logger_,
                         "Sending advertisement failed: " << ec.message());
        }
      });
}

void AdvSender::StartTimer() {
  timer_.expires_after(info_->adv_interval);

  auto weak_self = std::weak_ptr<AdvSender>(shared_from_this());
  timer_.async_wait([weak_self](auto ec) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!ec) {
      self->SendAdvertisement();
    } else {
      YOGI_LOG_ERROR(self->logger_, "Awaiting advertising timer expiry failed: "
                                        << ec.message());
    }
  });
}

}  // namespace detail
}  // namespace objects
