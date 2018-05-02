#include "adv_sender.h"
#include "../../../api/constants.h"
#include "../../../api/error.h"

#include <boost/endian/arithmetic.hpp>

namespace objects {
namespace detail {
namespace adv {

LoggerPtr AdvSender::logger_ = Logger::CreateStaticInternalLogger("Branch");

AdvSender::AdvSender(ContextPtr context,
                     const boost::asio::ip::udp::endpoint& adv_ep,
                     std::chrono::milliseconds adv_interval,
                     const boost::uuids::uuid& uuid,
                     const boost::asio::ip::tcp::endpoint& tcp_acceptor_ep)
    : context_(context),
      interval_(adv_interval),
      uuid_(uuid),
      tcp_acceptor_ep_(tcp_acceptor_ep),
      message_(MakeAdvMessage()),
      ep_(adv_ep),
      socket_(context->IoContext()),
      timer_(context->IoContext()) {
  SetupSocket();
}

void AdvSender::Start() { SendAdvertisement(); }

void AdvSender::SetupSocket() {
  boost::system::error_code ec;
  socket_.open(ep_.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }
}

std::vector<char> AdvSender::MakeAdvMessage() {
  std::vector<char> msg = {'Y', 'O', 'G', 'I', 0};
  msg.push_back(api::kVersionMajor);
  msg.push_back(api::kVersionMinor);
  msg.insert(msg.end(), uuid_.begin(), uuid_.end());
  boost::endian::big_uint16_t port = tcp_acceptor_ep_.port();
  msg.resize(msg.size() + 2);
  std::memcpy(msg.data() + msg.size() - 2, &port, 2);
  return msg;
}

void AdvSender::SendAdvertisement() {
  auto weak_self = std::weak_ptr<AdvSender>{shared_from_this()};
  socket_.async_send_to(
      boost::asio::buffer(message_), ep_, [weak_self](auto ec, auto) {
        auto self = weak_self.lock();
        if (!self) return;

        if (!ec) {
          self->StartTimer();
        } else if (ec != boost::asio::error::operation_aborted) {
          YOGI_LOG_ERROR(self->logger_,
                         "Sending advertisement failed: " << ec.message());
        }
      });
}

void AdvSender::StartTimer() {
  timer_.expires_after(interval_);

  auto weak_self = std::weak_ptr<AdvSender>(shared_from_this());
  timer_.async_wait([weak_self](auto ec) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!ec) {
      self->SendAdvertisement();
    } else if (ec != boost::asio::error::operation_aborted) {
      YOGI_LOG_ERROR(self->logger_, "Awaiting advertising timer expiry failed: "
                                        << ec.message());
    }
  });
}

}  // namespace adv
}  // namespace detail
}  // namespace objects
