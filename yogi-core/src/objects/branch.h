#pragma once

#include "../config.h"
#include "context.h"

#include <boost/uuid/uuid.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <vector>
#include <string>
#include <functional>

namespace objects {

class Branch : public api::ExposedObjectT<Branch, api::ObjectType::kBranch> {
 public:
  Branch(ContextPtr context, std::string name, std::string description,
         std::string net_name, std::string password, std::string path,
         std::string adv_address, int adv_port,
         std::chrono::milliseconds adv_interval);

  void Start();

  const boost::uuids::uuid& GetUuid() const {
    return uuid_;
  }

  std::string MakeInfo() const;
  void ForeachDiscoveredBranch(
      const std::function<void (const boost::uuids::uuid&)>& fn) const;

  void ForeachDiscoveredBranch(
      const std::function<void (const boost::uuids::uuid&, std::string)>& fn)
      const;

 private:
  std::vector<char> MakeAdvertisingMessage() const;
  void SetupAcceptor();
  void SetupAdvertising();
  void SendAdvertisement();
  void StartAdvertisingTimer();
  void ReceiveAdvertisement();
  void HandleReceivedAdvertisement();
  boost::asio::ip::tcp::endpoint MakeTcpEndpoint(int port);
  boost::asio::ip::udp::endpoint MakeUdpEndpoint(int port);

  const ContextPtr context_;
  const boost::uuids::uuid uuid_;
  const std::string name_;
  const std::string description_;
  const std::string net_name_;
  const std::string password_;
  const std::string path_;
  const boost::asio::ip::address adv_address_;
  const int adv_port_;
  const std::chrono::milliseconds adv_interval_;
  const boost::posix_time::ptime start_time_;

  boost::asio::ip::udp::endpoint adv_tx_endpoint_;
  boost::asio::ip::udp::socket adv_tx_socket_;
  boost::asio::steady_timer adv_tx_timer_;
  std::vector<char> adv_tx_message_;

  boost::asio::ip::udp::socket adv_rx_socket_;
  boost::asio::ip::udp::endpoint adv_rx_sender_ep_;
  std::vector<char> adv_rx_buffer_;

  boost::asio::ip::tcp::acceptor acceptor_;
};

typedef std::shared_ptr<Branch> BranchPtr;

}  // namespace objects
