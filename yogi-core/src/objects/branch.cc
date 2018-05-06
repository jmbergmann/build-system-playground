#include "branch.h"
#include "../api/constants.h"
#include "../api/error.h"
#include "../utils/system.h"
#include "../../../3rd_party/json/json.hpp"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/endian/arithmetic.hpp>
#include <sstream>

namespace objects {

const LoggerPtr Branch::logger_ = Logger::CreateStaticInternalLogger("Branch");

Branch::Branch(ContextPtr context, std::string name, std::string description,
               std::string net_name, std::string password, std::string path,
               std::string adv_address, int adv_port,
               std::chrono::nanoseconds adv_interval,
               std::chrono::nanoseconds timeout,
               std::chrono::nanoseconds retry_time)
    : context_(context),
      uuid_(boost::uuids::random_generator()()),
      name_(name),
      description_(description),
      net_name_(net_name),
      password_(password),
      path_(path),
      adv_ep_(boost::asio::ip::address::from_string(adv_address),
              static_cast<unsigned short>(adv_port)),
      adv_interval_(adv_interval),
      timeout_(timeout),
      retry_time_(retry_time),
      start_time_(utils::Timestamp::Now()),
      acceptor_(context_->IoContext()) {
  SetupAcceptor();
  SetupAdvertising();
  SetupQuerier();
}

void Branch::Start() {
  adv_sender_->Start();
  adv_receiver_->Start();
}

std::string Branch::MakeInfo() const {
  auto json = nlohmann::json{
      {"uuid", boost::uuids::to_string(uuid_)},
      {"name", name_},
      {"description", description_},
      {"net_name", net_name_},
      {"path", path_},
      {"hostname", utils::GetHostname()},
      {"pid", utils::GetProcessId()},
      {"advertising_address", adv_ep_.address().to_string()},
      {"advertising_port", adv_ep_.port()},
      {"advertising_interval", (float)adv_interval_.count() / 1000'000'000.0f},
      {"tcp_server_port", acceptor_.local_endpoint().port()},
      {"start_time", start_time_.ToJavaScriptString()},
      {"active_connections", 0},  // TODO
  };

  return json.dump();
}

void Branch::ForeachDiscoveredBranch(
    const std::function<void(const boost::uuids::uuid&)>& fn) const {}

void Branch::ForeachDiscoveredBranch(
    const std::function<void(const boost::uuids::uuid&, std::string)>& fn)
    const {}

void Branch::SetupAcceptor() {
  boost::system::error_code ec;

  auto protocol = adv_ep_.protocol() == boost::asio::ip::udp::v4()
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

  acceptor_.listen(acceptor_.max_listen_connections, ec);
  if (ec) {
    throw api::Error(YOGI_ERR_LISTEN_SOCKET_FAILED);
  }
}

void Branch::SetupAdvertising() {
  adv_sender_ = std::make_shared<detail::AdvSender>(
      context_, adv_ep_, adv_interval_, uuid_, acceptor_.local_endpoint());

  adv_receiver_ = std::make_shared<detail::AdvReceiver>(
      context_, adv_ep_, adv_sender_->GetMessageSize(),
      [this](auto& uuid, auto& ep) {
        this->OnAdvertisementReceived(uuid, ep);
      });
}

void Branch::SetupQuerier() {
  info_querier_ =
      std::make_shared<detail::InfoQuerier>(timeout_, retry_time_);
}

void Branch::OnAdvertisementReceived(
    const boost::uuids::uuid& uuid,
    const boost::asio::ip::tcp::endpoint& tcp_ep) {
  detail::BranchInfoPtr info;
  bool new_branch = false;
  {
    std::lock_guard<std::mutex> lock(branches_mutex_);
    auto& info_ref = branches_[uuid];
    if (!info_ref) {
      info_ref = std::make_shared<detail::BranchInfo>();
      info_ref->uuid = uuid;
      info_ref->tcp_ep = tcp_ep;
      new_branch = true;
    }

    info = info_ref;
  }

  if (new_branch) {
    YOGI_LOG_INFO(logger_, "New branch " << uuid << " discovered on "
                                         << tcp_ep.address());
  }

  std::lock_guard<std::mutex> lock(info->mutex);
  if (new_branch) {
    info_querier_->QueryBranch(info, [this, info](auto& socket) {
      this->OnQueryBranchSucceeded(info, socket);
    });
  }

  info->last_activity = utils::Timestamp::Now();
}

void Branch::OnQueryBranchSucceeded(const detail::BranchInfoPtr& info,
                            const utils::TimedTcpSocketPtr& socket) {
  std::lock_guard<std::mutex> lock(info->mutex);
  info->last_activity = utils::Timestamp::Now();
}

}  // namespace objects
