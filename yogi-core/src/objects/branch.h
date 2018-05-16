#pragma once

#include "../config.h"
#include "../utils/timestamp.h"
#include "../utils/socket.h"
#include "context.h"
#include "detail/branch/adv_receiver.h"
#include "detail/branch/adv_sender.h"
#include "detail/branch/branch_info.h"
#include "detail/branch/tcp_client.h"
#include "detail/branch/tcp_server.h"

#include <boost/functional/hash.hpp>
#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>

namespace objects {

class Branch : public api::ExposedObjectT<Branch, api::ObjectType::kBranch> {
 public:
  Branch(ContextPtr context, std::string name, std::string description,
         std::string net_name, std::string password, std::string path,
         std::string adv_address, int adv_port,
         std::chrono::nanoseconds adv_interval,
         std::chrono::nanoseconds timeout, std::chrono::nanoseconds retry_time);

  void Start();

  const boost::uuids::uuid& GetUuid() const { return info_->uuid; }

  std::string MakeInfo() const;
  void ForeachDiscoveredBranch(
      const std::function<void(const boost::uuids::uuid&)>& fn) const;

  void ForeachDiscoveredBranch(
      const std::function<void(const boost::uuids::uuid&, std::string)>& fn)
      const;

 private:
  typedef std::unordered_map<boost::uuids::uuid, detail::RemoteBranchInfoPtr,
                             boost::hash<boost::uuids::uuid>>
      BranchesMap;

  void SetupTcp();
  void SetupAdvertising();
  void OnAdvertisementReceived(const boost::uuids::uuid& uuid,
                               const boost::asio::ip::tcp::endpoint& tcp_ep);
  void OnConnectFinished(const api::Error& err,
                         const detail::RemoteBranchInfoPtr& info);
  void OnNewTcpConnection(const detail::RemoteBranchInfoPtr& branch);
  int GetNumActiveConnections() const;

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const detail::LocalBranchInfoPtr info_;
  const std::string password_;

  detail::AdvSenderPtr adv_sender_;
  detail::AdvReceiverPtr adv_receiver_;
  detail::TcpClientPtr tcp_client_;
  detail::TcpServerPtr tcp_server_;

  BranchesMap branches_;
  mutable std::mutex branches_mutex_;
};

typedef std::shared_ptr<Branch> BranchPtr;

}  // namespace objects
