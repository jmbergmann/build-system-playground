#include "adv_receiver.h"
#include "../../../api/constants.h"
#include "../../../api/error.h"
#include "../../../utils/serialize.h"

#include <boost/endian/arithmetic.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace objects {
namespace detail {

const LoggerPtr AdvReceiver::logger_ =
    Logger::CreateStaticInternalLogger("Branch");

AdvReceiver::AdvReceiver(ContextPtr context, LocalBranchInfoPtr info,
                         ObserverFn&& observer_fn)
    : context_(context),
      info_(info),
      observer_fn_(observer_fn),
      socket_(context->IoContext()),
      buffer_(BranchInfo::GetAdvertisingMessageSize() + 1) {
  SetupSocket();
}

void AdvReceiver::Start() { ReceiveAdvertisement(); }

void AdvReceiver::SetupSocket() {
  boost::system::error_code ec;
  socket_.open(info_->adv_ep.protocol(), ec);
  if (ec) throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);

  socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true), ec);
  if (ec) throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);

  socket_.bind(boost::asio::ip::udp::endpoint(info_->adv_ep.protocol(),
                                              info_->adv_ep.port()),
               ec);
  if (ec) throw api::Error(YOGI_ERR_BIND_SOCKET_FAILED);

  socket_.set_option(
      boost::asio::ip::multicast::join_group(info_->adv_ep.address()), ec);
  if (ec) throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);
}

void AdvReceiver::ReceiveAdvertisement() {
  auto weak_self = std::weak_ptr<AdvReceiver>{shared_from_this()};
  socket_.async_receive_from(
      boost::asio::buffer(buffer_), sender_ep_,
      [weak_self](auto ec, auto size) {
        auto self = weak_self.lock();
        if (!self) return;

        if (!ec) {
          if (size == BranchInfo::GetAdvertisingMessageSize()) {
            self->HandleReceivedAdvertisement();
            self->ReceiveAdvertisement();
          } else {
            YOGI_LOG_WARNING(self->logger_,
                             "Unexpected advertising message size ("
                                 << size << " bytes) received");
          }
        } else if (ec != boost::asio::error::operation_aborted) {
          YOGI_LOG_ERROR(
              self->logger_,
              "Failed to receive advertising message: " << ec.message());
        }
      });
}

void AdvReceiver::HandleReceivedAdvertisement() {
  if (auto err = BranchInfo::CheckAdvertisingMessageValidity(buffer_)) {
    YOGI_LOG_WARNING(logger_, "Invalid advertising message received from "
                                  << sender_ep_.address() << " :" << err);
    return;
  }

  auto it = buffer_.cbegin() + BranchInfo::GetAdvertisingMessageHeaderSize();

  boost::uuids::uuid uuid;
  utils::Deserialize(&uuid, buffer_, &it);
  unsigned short tcp_port;
  utils::Deserialize(&tcp_port, buffer_, &it);

  YOGI_LOG_TRACE(logger_, "Received advertising message for "
                              << uuid << " from " << sender_ep_.address());

  observer_fn_(uuid,
               boost::asio::ip::tcp::endpoint(sender_ep_.address(), tcp_port));
}

}  // namespace detail
}  // namespace objects
