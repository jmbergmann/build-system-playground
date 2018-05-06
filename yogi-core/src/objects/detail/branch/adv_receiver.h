#pragma once

#include "../../../config.h"
#include "../../context.h"
#include "../../logger.h"

#include <boost/uuid/uuid.hpp>
#include <boost/asio.hpp>
#include <vector>
#include <memory>

namespace objects {
namespace detail {
namespace branch {

class AdvReceiver : public std::enable_shared_from_this<AdvReceiver> {
 public:
  typedef std::function<void(const boost::uuids::uuid& uuid,
                             const boost::asio::ip::tcp::endpoint& ep)>
      ObserverFn;

  AdvReceiver(ContextPtr context, const boost::asio::ip::udp::endpoint& adv_ep,
              std::size_t adv_msg_size, ObserverFn&& observer_fn);
  void Start();

 private:
  void SetupSocket();
  void ReceiveAdvertisement();
  void HandleReceivedAdvertisement();

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const boost::asio::ip::udp::endpoint adv_ep_;
  const std::size_t adv_msg_size_;
  const ObserverFn observer_fn_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint sender_ep_;
  std::vector<char> buffer_;
};

typedef std::shared_ptr<AdvReceiver> AdvReceiverPtr;

}  // namespace branch
}  // namespace detail
}  // namespace objects
