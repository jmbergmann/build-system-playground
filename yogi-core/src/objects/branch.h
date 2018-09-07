#pragma once

#include "../config.h"
#include "context.h"
#include "detail/branch/connection_manager.h"

namespace objects {

class Branch : public api::ExposedObjectT<Branch, api::ObjectType::kBranch> {
 public:
  typedef detail::ConnectionManager::BranchEvents BranchEvents;
  typedef detail::ConnectionManager::BranchEventHandler BranchEventHandler;
  typedef detail::ConnectionManager::BranchInfoStringsList
      BranchInfoStringsList;

  Branch(ContextPtr context, std::string name, std::string description,
         std::string net_name, std::string password, std::string path,
         const boost::asio::ip::udp::endpoint& adv_ep,
         std::chrono::nanoseconds adv_interval,
         std::chrono::nanoseconds timeout, bool ghost_mode);

  void Start();

  const boost::uuids::uuid& GetUuid() const { return info_->GetUuid(); }
  std::string MakeInfoString() const;
  BranchInfoStringsList MakeConnectedBranchesInfoStrings() const;

  void AwaitEvent(BranchEvents events, BranchEventHandler handler) {
    connection_manager_->AwaitEvent(events, handler);
  }
  void CancelAwaitEvent() { connection_manager_->CancelAwaitEvent(); }

 private:
  void OnConnectionChanged(const api::Error& err,
                           const detail::BranchConnectionPtr& conn);
  void OnMessageReceived(const utils::ByteVector& msg, std::size_t size,
                         const detail::BranchConnectionPtr& conn);

  static const LoggerPtr logger_;

  const ContextPtr context_;
  const detail::ConnectionManagerPtr connection_manager_;
  const detail::BranchInfoPtr info_;
};

typedef std::shared_ptr<Branch> BranchPtr;

}  // namespace objects
