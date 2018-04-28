#include "adv_receiver.h"
#include "../../api/constants.h"
#include "../../api/error.h"

namespace objects {
namespace detail {

AdvertisingReceiver::AdvertisingReceiver(
    ContextPtr context, const boost::asio::ip::udp::endpoint& adv_ep,
    std::size_t adv_msg_size, ObserverFn observer_fn)
    : context_(context),
      adv_msg_size_(adv_msg_size),
      observer_fn_(observer_fn),
      adv_ep_(adv_ep),
      socket_(context->IoContext()),
      buffer_(adv_msg_size + 1) {
  SetupSocket();
}

void AdvertisingReceiver::Start() { ReceiveAdvertisement(); }

void AdvertisingReceiver::SetupSocket() {
  boost::system::error_code ec;
  socket_.open(adv_ep_.protocol(), ec);
  if (ec) throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);

  socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
  if (ec) throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);

  socket_.bind(
      boost::asio::ip::udp::endpoint(adv_ep_.protocol(), adv_ep_.port()), ec);
  if (ec) throw api::Error(YOGI_ERR_BIND_SOCKET_FAILED);

  socket_.set_option(boost::asio::ip::multicast::join_group(adv_ep_.address()),
                     ec);
  if (ec) throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);
}

void AdvertisingReceiver::ReceiveAdvertisement() {
  auto weak_self = std::weak_ptr<AdvertisingReceiver>{shared_from_this()};
  socket_.async_receive_from(
      boost::asio::buffer(buffer_), sender_ep_,
      [weak_self](auto ec, auto size) {
        auto self = weak_self.lock();
        if (!self) return;

        if (!ec) {
          if (size == self->adv_msg_size_) {
            self->HandleReceivedAdvertisement();
            self->ReceiveAdvertisement();
          } else {
            // TODO: logging error (unexpected adv size received)
          }
        } else if (ec != boost::asio::error::operation_aborted) {
          // TODO: logging error
        }
      });
}

void AdvertisingReceiver::HandleReceivedAdvertisement() {
  if (std::memcmp(buffer_.data(), "YOGI", 5) != 0) {
    // TODO: logging: Adv received but magic prefix is wrong
  }

  if (buffer_[5] != api::kVersionMajor || buffer_[6] != api::kVersionMinor) {
    // TODO: logging: Adv received but versions incompatible
  }

  boost::uuids::uuid uuid;
  std::copy_n(buffer_.begin() + 7, 16, uuid.begin());

  boost::endian::big_uint16_t tcp_port;
  std::memcpy(&tcp_port, buffer_.data() + 23, 2);

  // TODO: logging success
  observer_fn_(uuid, tcp_port);
}

}  // namespace detail
}  // namespace objects
