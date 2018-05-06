#pragma once

#include "../../../config.h"
#include "../../context.h"
#include "../../logger.h"

#include <boost/uuid/uuid.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <vector>
#include <memory>

namespace objects {
namespace detail {

class AdvSender : public std::enable_shared_from_this<AdvSender> {
 public:
  AdvSender(ContextPtr context, const boost::asio::ip::udp::endpoint& adv_ep,
            std::chrono::nanoseconds adv_interval,
            const boost::uuids::uuid& uuid,
            const boost::asio::ip::tcp::endpoint& tcp_acceptor_ep);
  void Start();
  std::size_t GetMessageSize() const { return message_.size(); }

 private:
  void SetupSocket();
  std::vector<char> MakeAdvMessage();
  void SendAdvertisement();
  void StartTimer();

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const std::chrono::nanoseconds interval_;
  const boost::uuids::uuid uuid_;
  const boost::asio::ip::tcp::endpoint tcp_acceptor_ep_;
  const std::vector<char> message_;
  boost::asio::ip::udp::endpoint ep_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::steady_timer timer_;
};

typedef std::shared_ptr<AdvSender> AdvSenderPtr;

}  // namespace detail
}  // namespace objects
