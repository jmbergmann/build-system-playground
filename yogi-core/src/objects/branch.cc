#include "branch.h"
#include "../api/constants.h"
#include "../api/error.h"
#include "../utils/system.h"
#include "../../../3rd_party/json/json.hpp"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/endian/arithmetic.hpp>
#include <sstream>
#include <algorithm>

namespace objects {

const LoggerPtr Branch::logger_ = Logger::CreateStaticInternalLogger("Branch");

Branch::Branch(ContextPtr context, std::string name, std::string description,
               std::string net_name, std::string password, std::string path,
               std::string adv_address, int adv_port,
               std::chrono::nanoseconds adv_interval,
               std::chrono::nanoseconds timeout)
    : context_(context),
      info_(std::make_shared<detail::LocalBranchInfo>()),
      password_(password) {
  info_->uuid = boost::uuids::random_generator()();
  info_->name = name;
  info_->description = description;
  info_->net_name = net_name;
  info_->path = path;
  info_->timeout = timeout;
  info_->adv_ep = boost::asio::ip::udp::endpoint(
      boost::asio::ip::address::from_string(adv_address),
      static_cast<unsigned short>(adv_port));
  info_->adv_interval = adv_interval;

  SetupTcp();
  SetupAdvertising();
}

void Branch::Start() {
  tcp_server_->Start();
  adv_sender_->Start();
  adv_receiver_->Start();
}

std::string Branch::MakeInfo() const {
  auto json = info_->ToJson();
  json["active_connections"] = GetNumActiveConnections();
  return json.dump();
}

void Branch::ForeachDiscoveredBranch(
    const std::function<void(const boost::uuids::uuid&)>& fn) const {}

void Branch::ForeachDiscoveredBranch(
    const std::function<void(const boost::uuids::uuid&, std::string)>& fn)
    const {}

void Branch::SetupTcp() {
  tcp_client_ = std::make_shared<detail::TcpClient>(context_, info_, [&](auto&& branch) {
    OnNewTcpConnection(std::move(branch));
  });

  tcp_server_ = std::make_shared<detail::TcpServer>(context_, info_, [&](auto&& branch) {
    OnNewTcpConnection(std::move(branch));
  });
}

void Branch::SetupAdvertising() {
  adv_sender_ =
      std::make_shared<detail::AdvSender>(context_, info_);

  adv_receiver_ = std::make_shared<detail::AdvReceiver>(
      context_, info_, [this](auto& uuid, auto& ep) {
        this->OnAdvertisementReceived(uuid, ep);
      });
}

void Branch::OnAdvertisementReceived(
    const boost::uuids::uuid& uuid,
    const boost::asio::ip::tcp::endpoint& tcp_ep) {
  detail::RemoteBranchInfoPtr branch;
  bool new_branch = false;
  {
    std::lock_guard<std::mutex> lock(branches_mutex_);
    auto& branch_ref = branches_[uuid];
    if (!branch_ref) {
      branch_ref = std::make_shared<detail::RemoteBranchInfo>();
      branch_ref->uuid = uuid;
      branch_ref->tcp_ep = tcp_ep;
      new_branch = true;
    }

    branch = branch_ref;
  }

  if (new_branch) {
    YOGI_LOG_INFO(logger_, "New branch " << uuid << " discovered on "
                                         << tcp_ep.address());
  }

  std::lock_guard<std::mutex> lock(branch->mutex);
  branch->last_activity = utils::Timestamp::Now();

  if (new_branch) {
    auto weak_self = MakeWeakPtr();
    tcp_client_->Connect(branch->tcp_ep, [weak_self, branch](auto& err) {
      auto self = weak_self.lock();
      if (!self) return;

      self->OnConnectFinished(err, branch);
    });
  }
}

void Branch::OnConnectFinished(const api::Error& err,
                               const detail::RemoteBranchInfoPtr& branch) {
  std::lock_guard<std::mutex> lock(branch->mutex);
  branch->last_activity = utils::Timestamp::Now();
  YOGI_TRACE;
}

void Branch::OnNewTcpConnection(const detail::RemoteBranchInfoPtr& branch) {
  YOGI_TRACE;
}

int Branch::GetNumActiveConnections() const {
  std::lock_guard<std::mutex> lock(branches_mutex_);
  return static_cast<int>(
      std::count_if(branches_.begin(), branches_.end(),
                    [](auto& branch) { return branch.second->connected; }));
}

}  // namespace objects
