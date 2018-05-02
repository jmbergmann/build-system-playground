#pragma once

#include "../config.h"
#include "../utils/timestamp.h"
#include "context.h"
#include "detail/adv/adv_receiver.h"
#include "detail/adv/adv_sender.h"

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

  const boost::uuids::uuid& GetUuid() const { return uuid_; }

  std::string MakeInfo() const;
  void ForeachDiscoveredBranch(
      const std::function<void(const boost::uuids::uuid&)>& fn) const;

  void ForeachDiscoveredBranch(
      const std::function<void(const boost::uuids::uuid&, std::string)>& fn)
      const;

 private:
  void SetupAcceptor();
  void SetupAdvertising();
  void OnAdvertisementReceived(const boost::uuids::uuid& uuid,
                               const boost::asio::ip::address& address,
                               unsigned short tcp_port);

  const ContextPtr context_;
  const boost::uuids::uuid uuid_;
  const std::string name_;
  const std::string description_;
  const std::string net_name_;
  const std::string password_;
  const std::string path_;
  const boost::asio::ip::udp::endpoint adv_ep_;
  const std::chrono::milliseconds adv_interval_;
  const utils::Timestamp start_time_;

  detail::adv::AdvSenderPtr adv_sender_;
  detail::adv::AdvReceiverPtr adv_receiver_;

  boost::asio::ip::tcp::acceptor acceptor_;
};

typedef std::shared_ptr<Branch> BranchPtr;

}  // namespace objects
