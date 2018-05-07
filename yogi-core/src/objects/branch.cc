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
               std::chrono::nanoseconds timeout,
               std::chrono::nanoseconds retry_time)
    : context_(context),
      info_(std::make_shared<detail::LocalBranchInfo>(
          boost::uuids::random_generator()())),
      acceptor_(context_->IoContext()) {
  info_->name = name;
  info_->description = description;
  info_->net_name = net_name;
  info_->path = path;
  info_->timeout = timeout;
  info_->retry_time = retry_time;
  info_->adv_ep = boost::asio::ip::udp::endpoint(
      boost::asio::ip::address::from_string(adv_address),
      static_cast<unsigned short>(adv_port));
  info_->adv_interval = adv_interval;

  SetupAcceptor();
  SetupAdvertising();
  SetupQuerier();
}

void Branch::Start() {
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

void Branch::SetupAcceptor() {
  boost::system::error_code ec;

  auto protocol = info_->adv_ep.protocol() == boost::asio::ip::udp::v4()
                      ? boost::asio::ip::tcp::v4()
                      : boost::asio::ip::tcp::v6();
  auto ep = boost::asio::ip::tcp::endpoint(protocol, 0);

  acceptor_.open(ep.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }

  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);
  }

  acceptor_.bind(ep, ec);
  if (ec) {
    throw api::Error(YOGI_ERR_BIND_SOCKET_FAILED);
  }

  info_->tcp_ep = acceptor_.local_endpoint();

  acceptor_.listen(acceptor_.max_listen_connections, ec);
  if (ec) {
    throw api::Error(YOGI_ERR_LISTEN_SOCKET_FAILED);
  }
}

void Branch::SetupAdvertising() {
  adv_sender_ = std::make_shared<detail::AdvSender>(
      context_, info_->adv_ep, info_->adv_interval, info_->uuid,
      acceptor_.local_endpoint());

  adv_receiver_ = std::make_shared<detail::AdvReceiver>(
      context_, info_->adv_ep, adv_sender_->GetMessageSize(),
      [this](auto& uuid, auto& ep) {
        this->OnAdvertisementReceived(uuid, ep);
      });
}

void Branch::SetupQuerier() {
  info_querier_ =
      std::make_shared<detail::InfoQuerier>(info_->timeout, info_->retry_time);
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
      branch_ref = std::make_shared<detail::RemoteBranchInfo>(uuid);
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
  if (new_branch) {
    info_querier_->QueryBranch(branch, [this, branch]() {
      this->OnQueryBranchSucceeded(branch);
    });
  }

  branch->last_activity = utils::Timestamp::Now();
}

void Branch::OnQueryBranchSucceeded(const detail::RemoteBranchInfoPtr& branch) {
  std::lock_guard<std::mutex> lock(branch->mutex);
  branch->last_activity = utils::Timestamp::Now();
}

int Branch::GetNumActiveConnections() const {
  std::lock_guard<std::mutex> lock(branches_mutex_);
  return std::count_if(branches_.begin(), branches_.end(),
                       [](auto& branch) { return branch.second->connected; });
}

}  // namespace objects
