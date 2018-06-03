#include "branch.h"

namespace objects {

Branch::Branch(ContextPtr context, std::string name, std::string description,
               std::string net_name, std::string password, std::string path,
               const boost::asio::ip::udp::endpoint& adv_ep,
               std::chrono::nanoseconds adv_interval,
               std::chrono::nanoseconds timeout)
    : context_(context),
      connection_manager_(std::make_shared<detail::ConnectionManager>(
          context, password, adv_ep,
          [&](auto& err, auto connection) {
            OnConnectionChanged(err, connection);
          })),
      info_(detail::BranchInfo::CreateLocal(
          name, description, net_name, path,
          connection_manager_->GetTcpServerEndpoint(), timeout, adv_interval)) {
}

void Branch::Start() { connection_manager_->Start(info_); }

std::string Branch::MakeInfoString() const {
  auto json = info_->ToJson();
  auto& ep = connection_manager_->GetAdvertisingEndpoint();
  json["advertising_address"] = ep.address().to_string();
  json["advertising_port"] = ep.port();
  return json.dump();
}

Branch::BranchInfoStringsList Branch::MakeConnectedBranchesInfoStrings() const {
  return connection_manager_->MakeConnectedBranchesInfoStrings();
}

void Branch::OnConnectionChanged(const api::Error& err,
                                 detail::BranchConnectionPtr connection) {
  // todo
}

const LoggerPtr Branch::logger_ = Logger::CreateStaticInternalLogger("Branch");

}  // namespace objects
