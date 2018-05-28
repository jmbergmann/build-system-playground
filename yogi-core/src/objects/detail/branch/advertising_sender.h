#pragma once

#include "../../../config.h"
#include "../../context.h"
#include "../../logger.h"
#include "branch_info.h"

#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <memory>

namespace objects {
namespace detail {

class AdvertisingSender
    : public std::enable_shared_from_this<AdvertisingSender> {
 public:
  AdvertisingSender(ContextPtr context,
                    const boost::asio::ip::udp::endpoint& adv_ep);
  void Start(BranchInfoPtr info);

  const boost::asio::ip::udp::endpoint& GetEndpoint() const {
    return adv_ep_;
  }

 private:
  void SetupSocket();
  void SendAdvertisement();
  void StartTimer();

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const boost::asio::ip::udp::endpoint adv_ep_;
  BranchInfoPtr info_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::steady_timer timer_;
};

typedef std::shared_ptr<AdvertisingSender> AdvertisingSenderPtr;

}  // namespace detail
}  // namespace objects
