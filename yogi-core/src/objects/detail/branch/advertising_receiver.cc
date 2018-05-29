#include "advertising_receiver.h"
#include "../../../api/error.h"

#include <boost/asio/ip/multicast.hpp>

namespace objects {
namespace detail {

AdvertisingReceiver::AdvertisingReceiver(
    ContextPtr context, const boost::asio::ip::udp::endpoint& adv_ep,
    ObserverFn observer_fn)
    : context_(context),
      adv_ep_(adv_ep),
      observer_fn_(observer_fn),
      buffer_(std::make_shared<std::vector<char>>(
          BranchInfo::kAdvertisingMessageSize + 1)),
      socket_(context->IoContext()) {
  SetupSocket();
}

void AdvertisingReceiver::Start(BranchInfoPtr info) {
  YOGI_ASSERT(!info_);

  info_ = info;
  StartReceiveAdvertisement();
}

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

void AdvertisingReceiver::StartReceiveAdvertisement() {
  auto buffer = buffer_;
  auto weak_self = std::weak_ptr<AdvertisingReceiver>{shared_from_this()};
  socket_.async_receive_from(boost::asio::buffer(*buffer_), sender_ep_,
                             [weak_self, buffer](auto ec, auto bytes_received) {
                               auto self = weak_self.lock();
                               if (!self) return;

                               self->OnReceivedAdvertisementFinished(
                                   ec, bytes_received);
                             });
}

void AdvertisingReceiver::OnReceivedAdvertisementFinished(
    const boost::system::error_code& ec, std::size_t bytes_received) {
  if (ec) {
    YOGI_LOG_ERROR(
        logger_,
        info_ << "Failed to receive advertising message: " << ec.message()
              << ". No more advertising messages will be received.");
    return;
  }

  if (bytes_received != BranchInfo::kAdvertisingMessageSize) {
    YOGI_LOG_WARNING(logger_,
                     info_ << "Unexpected advertising message size received");
    StartReceiveAdvertisement();
    return;
  }

  boost::uuids::uuid uuid;
  unsigned short tcp_port;
  if (auto err = BranchInfo::DeserializeAdvertisingMessage(&uuid, &tcp_port,
                                                           *buffer_)) {
    YOGI_LOG_WARNING(logger_,
                     info_ << " Invalid advertising message received from "
                           << sender_ep_.address() << ": " << err);
    StartReceiveAdvertisement();
    return;
  }

  // Ignore advertising messages that we sent ourself
  if (uuid != info_->GetUuid()) {
    auto tcp_ep = boost::asio::ip::tcp::endpoint(sender_ep_.address(), tcp_port);
    observer_fn_(uuid, tcp_ep);
  }

  StartReceiveAdvertisement();
}

const LoggerPtr AdvertisingReceiver::logger_ =
    Logger::CreateStaticInternalLogger("Branch.AdvertisingReceiver");

}  // namespace detail
}  // namespace objects
