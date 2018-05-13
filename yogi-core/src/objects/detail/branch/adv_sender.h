#pragma once

#include "../../../config.h"
#include "../../context.h"
#include "../../logger.h"
#include "branch_info.h"

#include <boost/uuid/uuid.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <vector>
#include <memory>

namespace objects {
namespace detail {

class AdvSender : public std::enable_shared_from_this<AdvSender> {
 public:
  AdvSender(ContextPtr context, LocalBranchInfoPtr info);
  void Start();

 private:
  void SetupSocket();
  void SendAdvertisement();
  void StartTimer();

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const LocalBranchInfoPtr info_;
  std::vector<char> message_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::steady_timer timer_;
};

typedef std::shared_ptr<AdvSender> AdvSenderPtr;

}  // namespace detail
}  // namespace objects
