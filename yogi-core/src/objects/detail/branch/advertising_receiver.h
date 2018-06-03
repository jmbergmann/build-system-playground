#pragma once

#include "../../../config.h"
#include "../../context.h"
#include "../../logger.h"
#include "branch_info.h"

#include <boost/uuid/uuid.hpp>
#include <boost/asio/ip/udp.hpp>
#include <vector>
#include <memory>
#include <functional>

namespace objects {
namespace detail {

class AdvertisingReceiver
    : public std::enable_shared_from_this<AdvertisingReceiver> {
 public:
  typedef std::function<void(const boost::uuids::uuid& uuid,
                             const boost::asio::ip::tcp::endpoint& ep)>
      ObserverFn;

  AdvertisingReceiver(ContextPtr context,
                      const boost::asio::ip::udp::endpoint& adv_ep,
                      ObserverFn observer_fn);

  void Start(BranchInfoPtr info);

 private:
  void SetupSocket();
  void StartReceiveAdvertisement();
  void OnReceivedAdvertisementFinished(const boost::system::error_code& ec,
                                       std::size_t bytes_received);

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const boost::asio::ip::udp::endpoint adv_ep_;
  const ObserverFn observer_fn_;
  const utils::SharedByteVector buffer_;
  BranchInfoPtr info_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint sender_ep_;
};

typedef std::shared_ptr<AdvertisingReceiver> AdvertisingReceiverPtr;

}  // namespace detail
}  // namespace objects
